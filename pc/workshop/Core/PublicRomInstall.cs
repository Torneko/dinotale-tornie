using System.Security.Cryptography;
using System.Text.Json;
namespace YoshiWorkshop;
public sealed class PublicContentManifest {
    public int Format {get;set;}public string Version {get;set;}="";public int SourceSize {get;set;}public int TargetSize {get;set;}
    public string SourceSha256 {get;set;}="";public string TargetSha256 {get;set;}="";public string PatchSha256 {get;set;}="";public string EngineSha256 {get;set;}="";
}
public static class PublicRomInstall {
    static string Hash(byte[] bytes)=>Convert.ToHexStringLower(SHA256.HashData(bytes));
    public static bool Available(string root)=>File.Exists(Path.Combine(root,"distribution/content-patch.json"));
    public static bool Needed(string root)=>Available(root) && (!File.Exists(Path.Combine(root,"build/native-special/workshop-content.bin")) || !File.Exists(Path.Combine(root,"profiles/content-personal/yoshisstory.n64.us.z64")));
    public static byte[] Normalize(byte[] bytes){
        if(bytes.Length!=16_777_216)throw new InvalidDataException("Une ROM originale USA de Yoshi’s Story (16 Mio) est nécessaire. / An original USA Yoshi’s Story ROM (16 MiB) is required.");
        byte[] output=(byte[])bytes.Clone();
        if(output.AsSpan(0,4).SequenceEqual(new byte[]{0x37,0x80,0x40,0x12})){for(int i=0;i<output.Length;i+=2)(output[i],output[i+1])=(output[i+1],output[i]);}
        else if(output.AsSpan(0,4).SequenceEqual(new byte[]{0x40,0x12,0x37,0x80})){for(int i=0;i<output.Length;i+=4){(output[i],output[i+3])=(output[i+3],output[i]);(output[i+1],output[i+2])=(output[i+2],output[i+1]);}}
        if(Hash(output)!=VanillaRom.ExpectedSha256)throw new InvalidDataException("Cette ROM n’est pas l’originale USA attendue. Les ROM déjà modifiées ne conviennent pas. / This is not the expected original USA ROM; patched ROMs are not accepted.");return output;
    }
    public static string Install(string root,string originalFile){
        root=Path.TrimEndingDirectorySeparator(Path.GetFullPath(root));string distribution=Path.Combine(root,"distribution"),manifestPath=Path.Combine(distribution,"content-patch.json");if(!Available(root) || new FileInfo(manifestPath).Length>8192)throw new InvalidDataException("Manifeste d’installation absent ou invalide.");
        var m=JsonSerializer.Deserialize<PublicContentManifest>(File.ReadAllText(manifestPath),ProjectStore.Json)??throw new InvalidDataException("Manifeste invalide.");
        if(m.Format!=1 || m.SourceSha256!=VanillaRom.ExpectedSha256 || m.SourceSize!=16_777_216 || m.TargetSize!=16_777_216)throw new InvalidDataException("Contenu incompatible.");
        if(new FileInfo(originalFile).Length!=m.SourceSize)throw new InvalidDataException("La ROM doit avoir une taille de 16 Mio. / The ROM must be 16 MiB.");
        byte[] original=Normalize(File.ReadAllBytes(originalFile));string patchPath=Path.Combine(distribution,"content.bps");if(new FileInfo(patchPath).Length>4_194_304)throw new InvalidDataException("Patch trop volumineux.");byte[] patch=File.ReadAllBytes(patchPath);
        string engine=Path.Combine(root,"build/native-special/DinoTale.exe");if(Hash(patch)!=m.PatchSha256 || !File.Exists(engine) || Hash(File.ReadAllBytes(engine))!=m.EngineSha256)throw new InvalidDataException("Les fichiers du paquet sont incomplets ou modifiés. / Package files are incomplete or modified.");
        byte[] content=BpsPatch.Apply(original,patch);if(content.Length!=m.TargetSize || Hash(content)!=m.TargetSha256)throw new InvalidDataException("Le contenu préparé ne correspond pas à cette version.");
        var files=new[]{(Path.Combine(root,"profiles/content-personal/yoshisstory.n64.us.z64"),original,m.SourceSha256),(Path.Combine(root,"build/native-special/workshop-content.bin"),content,m.TargetSha256)};
        // Verify every existing destination first; a repair must never replace unrelated files or saves.
        foreach(var (path,bytes,hash) in files)if(File.Exists(path) && Hash(File.ReadAllBytes(path))!=hash)throw new InvalidDataException("Ce dossier contient déjà un autre contenu. Utilise une nouvelle copie du paquet ; les fichiers existants sont conservés. / This folder contains different content. Use a fresh package; existing files are preserved.");
        foreach(var (path,bytes,hash) in files){if(File.Exists(path))continue;string resolved=Path.GetFullPath(path);if(!resolved.StartsWith(root+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase))throw new InvalidDataException("Destination invalide.");Directory.CreateDirectory(Path.GetDirectoryName(path)!);string temp=path+"."+Guid.NewGuid().ToString("N")+".tmp";try{File.WriteAllBytes(temp,bytes);if(Hash(File.ReadAllBytes(temp))!=hash)throw new IOException("La copie locale n’a pas pu être vérifiée.");File.Move(temp,path,false);}finally{if(File.Exists(temp))File.Delete(temp);}}
        return m.Version;
    }
}
