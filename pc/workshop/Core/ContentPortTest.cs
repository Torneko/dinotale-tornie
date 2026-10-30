using System.Diagnostics;
using System.Security.Cryptography;
using System.Text.Json;

namespace YoshiWorkshop;

// This profile is deliberately separate from imported/user progression. The
// experimental engine understands the mod's EEPROM format; the stable vanilla
// engine understands the converted native format. They must never share saves.
public sealed class ContentPortTest(string root) {
    public const string FixtureSha256="bc726e919c59294e27e33352dca0c1c5f77b99e313d75051129781c026340dc5";
    public Process? Running { get; private set; }
    public bool IsRunning=>Running is {HasExited:false};
    static string Hash(string path)=>Convert.ToHexStringLower(SHA256.HashData(File.ReadAllBytes(path)));
    string Runtime(){
        var special=new SpecialContentRunner(root);
        if(special.Available){special.Verify();return special.RuntimeDirectory;}
        return Path.Combine(root,"build","native-content");
    }
    public string Prepare(bool personal=false){
        string runtime=Runtime();
        using var manifest=JsonDocument.Parse(File.ReadAllText(Path.Combine(runtime,"engine-build.json")));
        foreach(var file in new[]{("DinoTale.exe","exeSha256"),("workshop-content.bin","contentSha256")})
            if(Hash(Path.Combine(runtime,file.Item1))!=manifest.RootElement.GetProperty(file.Item2).GetString())throw new InvalidDataException("Le moteur de test diffère de son rapport de compilation.");
        string original=Path.Combine(root,"..","work","yoshis-story-decomp","baseroms","us","baserom.z64");
        if(Hash(original)!=VanillaRom.ExpectedSha256)throw new InvalidDataException("La ROM originale USA est requise.");
        string profile=Path.Combine(root,"profiles",personal?"content-personal":"content-port-test");Directory.CreateDirectory(profile);
        string save=Path.Combine(profile,"saves","yoshisstory.n64.us.bin");
        Directory.CreateDirectory(Path.GetDirectoryName(save)!);
        if(personal)ContentSaveSeed.Initialize(root,profile);
        else {
            string fixture=Path.Combine(root,"..","work","experiments","extension-v9-2","test-tous-yoshis.eep");
            if(Hash(fixture)!=FixtureSha256)throw new InvalidDataException("La sauvegarde de test des 14 Yoshis est incorrecte.");
            if(!File.Exists(save))File.Copy(fixture,save);
        }
        File.Copy(original,Path.Combine(profile,"yoshisstory.n64.us.z64"),true);
        File.WriteAllText(Path.Combine(profile,"portable.txt"),"");
        foreach(string source in Directory.GetFiles(Path.Combine(runtime,"assets"),"*",SearchOption.AllDirectories)){
            string target=Path.Combine(profile,"assets",Path.GetRelativePath(Path.Combine(runtime,"assets"),source));
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);File.Copy(source,target,true);
        }
        bool hybrids=Path.GetFileName(runtime).StartsWith("native-special",StringComparison.Ordinal);
        File.WriteAllText(Path.Combine(profile,"content-profile.json"),JsonSerializer.Serialize(new{purpose=personal?"Personal progression with earned Yoshis":"14-Yoshi PC port manual verification",testUnlocks=!personal,usesPersonalProgress=personal,saveFormat="v9.2",newFruitGameplay=hybrids,hybridFruitChancePercent=hybrids?25:0,storyColumnRewards=hybrids,newMaskassGameplay=false},ProjectStore.Json));
        return profile;
    }
    public void Launch(bool personal=false){
        if(IsRunning)throw new InvalidOperationException("Le test des Yoshis est déjà ouvert.");
        string profile=Prepare(personal);
        var start=new ProcessStartInfo(Path.Combine(Runtime(),"DinoTale.exe")){WorkingDirectory=profile,UseShellExecute=false};
        if(personal && File.Exists(Path.Combine(root,"portable-package.json")))start.ArgumentList.Add("--workshop-main-menu");
        else {start.ArgumentList.Add("--game");start.ArgumentList.Add("yoshisstory");}
        Running=Process.Start(start);
    }
}

public static class ContentSaveSeed {
    public static bool Initialize(string root,string profile){
        string target=Path.Combine(profile,"saves","yoshisstory.n64.us.bin");
        if(File.Exists(target))return false;
        byte[] seed=File.ReadAllBytes(Path.Combine(root,"save-imports","content-default.bin"));
        using var report=JsonDocument.Parse(File.ReadAllText(Path.Combine(root,"save-imports","content-import.json")));
        var info=report.RootElement;
        if(seed.Length!=2048 || Convert.ToHexStringLower(SHA256.HashData(seed))!=info.GetProperty("seedSha256").GetString()
            || info.GetProperty("testUnlocksUsed").GetBoolean() || !info.GetProperty("earnedUnlocksPreserved").GetBoolean()
            || !seed.AsSpan(0,1024).SequenceEqual(seed.AsSpan(1024)) || !seed.AsSpan(1020,4).SequenceEqual(new byte[]{0x81,0x31,0x75,0x39}))
            throw new InvalidDataException("La sauvegarde des Yoshis débloqués ne correspond pas à son rapport d’import.");
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        using var output=new FileStream(target,FileMode.CreateNew,FileAccess.Write,FileShare.None);output.Write(seed);output.Flush(true);return true;
    }
}
