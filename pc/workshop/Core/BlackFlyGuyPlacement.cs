namespace YoshiWorkshop;

public sealed partial class VanillaRom {
    // 45E3 is normally created by the 45E5 spiked-block assembly. The native
    // source must also preload its resources independently of that placement.
    public bool CanPlaceBlackFlyGuy(int room){
        if(!NativeRooms.Known(room) || !ReadRoom(room).Objects.Any(p=>p.ObjectId==0x45E5))return false;
        int header=Bank(U32(0xA84A0+room*8+4));bool found=false;
        for(int offset=0x60;offset+2<=0x118;offset+=2){
            ushort id=U16(header+offset);
            if(id==0xFFFF)return found;
            if(id==0x45E3)found=true;
        }
        return false;
    }
}
