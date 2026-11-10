using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Security.Cryptography;
using System.Text.Json;

// Technical conversion only: preserve the approved artwork, fit it into the
// native sprite frame, then encode the game's bottom-up CI8 / RGBA5551 format.
if(args.Length!=1)throw new ArgumentException("Expected the absolute pc folder.");
string root=Path.GetFullPath(args[0]);
string output=Path.Combine(root,"build","special-assets");Directory.CreateDirectory(output);
string[] names=["peche","mure","bleuet","poire-verte","prune","kiwi"];
var report=new List<object>();
foreach(string name in names){
    string source=Path.Combine(root,"assets","fruits",name+(name is "peche" or "mure"?"-source.png":"-matched.png"));
    using var original=new Bitmap(source);
    int left=original.Width,top=original.Height,right=-1,bottom=-1;
    for(int y=0;y<original.Height;y++)for(int x=0;x<original.Width;x++)if(original.GetPixel(x,y).A>=128){left=Math.Min(left,x);right=Math.Max(right,x);top=Math.Min(top,y);bottom=Math.Max(bottom,y);}
    if(right<left)throw new InvalidDataException("Empty transparent asset: "+source);
    using var sprite=new Bitmap(24,21,PixelFormat.Format32bppArgb);
    using(var graphics=Graphics.FromImage(sprite)){
        graphics.Clear(Color.Transparent);graphics.InterpolationMode=InterpolationMode.HighQualityBicubic;graphics.CompositingMode=CompositingMode.SourceCopy;graphics.PixelOffsetMode=PixelOffsetMode.HighQuality;
        float scale=Math.Min(22f/(right-left+1),19f/(bottom-top+1));
        float width=(right-left+1)*scale,height=(bottom-top+1)*scale;
        graphics.DrawImage(original,new RectangleF((24-width)/2,(21-height)/2,width,height),new RectangleF(left,top,right-left+1,bottom-top+1),GraphicsUnit.Pixel);
    }
    // Match the PC turquoise Yoshi's subtle 5% blend toward green. Recolor
    // only cyan/blue fruit pixels; retain its violet calyx and transparency.
    if(name=="bleuet")for(int y=0;y<21;y++)for(int x=0;x<24;x++){
        var c=sprite.GetPixel(x,y);
        if(c.A<128 || c.GetHue()<145 || c.GetHue()>235 || c.GetSaturation()<=.13)continue;
        double high=Math.Max(c.R,Math.Max(c.G,c.B)),low=Math.Min(c.R,Math.Min(c.G,c.B));
        sprite.SetPixel(x,y,Color.FromArgb(c.A,(int)Math.Round(c.R*.95+low*.05),(int)Math.Round(c.G*.95+high*.05),(int)Math.Round(c.B*.95+low*.05)));
    }
    var pixels=new byte[24*21];
    // Keep at most 255 opaque RGB555 colors plus the transparent entry.
    var frequencies=new Dictionary<ushort,int>();
    for(int y=0;y<21;y++)for(int x=0;x<24;x++){
        Color c=sprite.GetPixel(x,y);ushort value=c.A<128?(ushort)0:(ushort)((c.R>>3)<<11|(c.G>>3)<<6|(c.B>>3)<<1|1);
        if(value!=0)frequencies[value]=frequencies.GetValueOrDefault(value)+1;
    }
    List<ushort> colors=[0,..frequencies.OrderByDescending(x=>x.Value).ThenBy(x=>x.Key).Take(255).Select(x=>x.Key)];
    int Distance(ushort a,ushort b){int r=(a>>11)-(b>>11),g=((a>>6)&31)-((b>>6)&31),bl=((a>>1)&31)-((b>>1)&31);return r*r+g*g+bl*bl;}
    for(int y=0;y<21;y++)for(int x=0;x<24;x++){
        Color c=sprite.GetPixel(x,20-y);ushort value=c.A<128?(ushort)0:(ushort)((c.R>>3)<<11|(c.G>>3)<<6|(c.B>>3)<<1|1);
        int index=value==0?0:Enumerable.Range(1,colors.Count-1).MinBy(i=>Distance(value,colors[i]));pixels[y*24+x]=(byte)index;
        ushort q=colors[index];sprite.SetPixel(x,20-y,index==0?Color.Transparent:Color.FromArgb(255,((q>>11)&31)*255/31,((q>>6)&31)*255/31,((q>>1)&31)*255/31));
    }
    var palette=new byte[512];for(int i=0;i<colors.Count;i++){palette[i*2]=(byte)(colors[i]>>8);palette[i*2+1]=(byte)colors[i];}
    // Native fruits have nine frame slots. The first content pass uses the
    // same approved view in each slot; native bobbing/rotation remains active.
    File.WriteAllBytes(Path.Combine(output,name+".ci8"),Enumerable.Range(0,9).SelectMany(_=>pixels).ToArray());
    File.WriteAllBytes(Path.Combine(output,name+".rgba5551"),palette);
    sprite.Save(Path.Combine(output,name+"-24x21.png"),ImageFormat.Png);
    report.Add(new{name,source=Path.GetRelativePath(root,source),sourceSha256=Convert.ToHexStringLower(SHA256.HashData(File.ReadAllBytes(source))),greenBlendPercent=name=="bleuet"?5:0,width=24,height=21,frames=9,distinctColors=colors.Count,transparentPixels=pixels.Count(x=>x==0)});
}
File.WriteAllText(Path.Combine(output,"assets.json"),JsonSerializer.Serialize(report,new JsonSerializerOptions{WriteIndented=true})+"\n");
Console.WriteLine("Encoded six transparent fruits: CI8, nine 24x21 frames each, RGBA5551 palettes.");
