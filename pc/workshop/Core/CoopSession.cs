namespace YoshiWorkshop;

// Host/local authority only. Clients receive immutable snapshots and submit
// inputs; collection and health decisions remain with the authoritative world.
public sealed class CoopSession {
    public const int MaxEntities=16384;
    public const int MaxInputSequenceJump=4096;
    readonly object gate=new();
    readonly string sessionId=Guid.NewGuid().ToString("N");
    readonly CoopSessionIdentity identity;
    readonly CoopSessionMode mode;
    readonly Dictionary<string,CoopWorldEntity> entities=new(StringComparer.Ordinal);
    readonly TeamProgress progress=new();
    readonly int[] colors=new int[2],health=new int[2];
    readonly bool[] connected=new bool[2],ready=new bool[2];
    readonly double[] respawn=new double[2];
    readonly CoopInputFrame?[] inputs=new CoopInputFrame?[2];
    long revision;CoopSessionPhase phase=CoopSessionPhase.Lobby;
    public CoopSession(CoopSessionIdentity identity,CoopSessionMode mode=CoopSessionMode.Local,int firstColor=0,int secondColor=1){
        identity.Validate();if(!Enum.IsDefined(mode) || firstColor is <0 or >13 || secondColor is <0 or >13 || mode==CoopSessionMode.Local && !identity.Rules.AllowLocal || mode!=CoopSessionMode.Local && !identity.Rules.AllowNetwork)throw new InvalidDataException("Unsupported cooperative session mode/colors.");this.identity=identity;this.mode=mode;colors[0]=firstColor;colors[1]=secondColor;health[0]=health[1]=identity.Rules.MaxHealth;connected[0]=mode!=CoopSessionMode.Join;connected[1]=mode==CoopSessionMode.Local;
    }
    public CoopSessionIdentity Identity=>identity;
    public bool IsAuthority=>mode!=CoopSessionMode.Join;
    public CoopSessionSnapshot Snapshot {get{lock(gate)return SnapshotLocked();}}
    void Authority(){if(!IsAuthority)throw new InvalidOperationException("Only the host/local world may change session state.");}
    static void Slot(int slot){if(slot is <0 or >1)throw new InvalidDataException("Exactly two cooperative slots are available.");}
    void Bump()=>revision=checked(revision+1);
    CoopSessionSnapshot SnapshotLocked()=>new(sessionId,identity,phase,revision,progress.FruitCount,progress.DefeatedCount,Array.AsReadOnly(Enumerable.Range(0,2).Select(i=>new CoopPlayerSnapshot(i,colors[i],connected[i],ready[i],health[i],(int)Math.Ceiling(respawn[i]),inputs[i]?.Sequence??0,inputs[i])).ToArray()));
    public void AttachGuest(CoopSessionIdentity peer,int yoshiColor){Authority();peer.RequireCompatible(identity);if(yoshiColor is <0 or >13)throw new InvalidDataException("Yoshi color outside0..13.");lock(gate){if(mode!=CoopSessionMode.Host || connected[1] || phase!=CoopSessionPhase.Lobby)throw new InvalidOperationException("Guest slot unavailable.");connected[1]=true;colors[1]=yoshiColor;ready[1]=false;Bump();}}
    public void DetachGuest(){Authority();lock(gate){if(!connected[1])return;connected[1]=ready[1]=false;ready[0]=false;if(phase==CoopSessionPhase.Running)phase=CoopSessionPhase.Disconnected;Bump();}}
    public bool SetColor(int slot,int color){Authority();Slot(slot);if(color is <0 or >13)throw new InvalidDataException("Yoshi color outside0..13.");lock(gate){if(phase!=CoopSessionPhase.Lobby || !connected[slot])throw new InvalidOperationException("Player cannot change lobby color now.");if(colors[slot]==color)return false;colors[slot]=color;ready[slot]=false;Bump();return true;}}
    public bool SetReady(int slot,bool value,CoopSessionIdentity peer){Authority();Slot(slot);peer.RequireCompatible(identity);lock(gate){if(phase!=CoopSessionPhase.Lobby || !connected[slot])throw new InvalidOperationException("Player cannot ready now.");if(ready[slot]==value)return false;ready[slot]=value;Bump();return true;}}
    public bool StartGame(){Authority();lock(gate){if(phase!=CoopSessionPhase.Lobby || !connected.All(v=>v) || !ready.All(v=>v))return false;phase=CoopSessionPhase.Running;Bump();return true;}}
    public void RegisterEntities(IEnumerable<CoopWorldEntity> values){Authority();var batch=values.Take(MaxEntities+1).ToArray();if(batch.Length>MaxEntities)throw new InvalidDataException("Entity catalog too large.");foreach(var entity in batch)entity.Validate();if(batch.Select(e=>e.StableId).Distinct(StringComparer.Ordinal).Count()!=batch.Length)throw new InvalidDataException("Duplicate entity catalog IDs.");lock(gate){if(entities.Count+batch.Count(e=>!entities.ContainsKey(e.StableId))>MaxEntities || batch.Any(e=>entities.TryGetValue(e.StableId,out var old) && old!=e))throw new InvalidDataException("Conflicting or excessive authoritative entities.");foreach(var entity in batch)entities.TryAdd(entity.StableId,entity);}}
    public bool TryInput(int slot,CoopInputFrame input){Authority();Slot(slot);input.Validate();lock(gate){if(phase!=CoopSessionPhase.Running || !connected[slot])return false;long previous=inputs[slot]?.Sequence??0;if(input.Sequence<=previous || input.Sequence-previous>MaxInputSequenceJump)return false;inputs[slot]=input;Bump();return true;}}
    public CoopAuthoritativeEvent? CollectFruit(int slot,string stableId)=>Collect(slot,stableId,CoopEntityKind.Fruit);
    public CoopAuthoritativeEvent? DefeatEnemy(int slot,string stableId)=>Collect(slot,stableId,CoopEntityKind.Enemy);
    CoopAuthoritativeEvent? Collect(int slot,string id,CoopEntityKind kind){Authority();Slot(slot);CoopWorldEntity.ValidateId(id);lock(gate){if(phase!=CoopSessionPhase.Running || !connected[slot] || health[slot]==0 || !entities.TryGetValue(id,out var entity) || entity.Kind!=kind)return null;bool added=kind==CoopEntityKind.Fruit?progress.Collect(id):progress.Defeat(id);if(!added)return null;if(identity.Rules.CompletionMode==CoopCompletionMode.FruitTarget && progress.FruitCount>=identity.Rules.FruitTarget)phase=CoopSessionPhase.Completed;Bump();return new(revision,kind,id,slot,progress.FruitCount,progress.DefeatedCount);}}
    public bool SetHealth(int slot,int value){Authority();Slot(slot);if(value<0 || value>identity.Rules.MaxHealth)throw new InvalidDataException("Health outside configured limits.");lock(gate){if(phase!=CoopSessionPhase.Running || health[slot]==value)return false;health[slot]=value;respawn[slot]=0;if(value==0){if(health[1-slot]>0)respawn[slot]=identity.Rules.RespawnDelayMilliseconds;else{phase=CoopSessionPhase.GameOver;respawn[0]=respawn[1]=0;}}Bump();return true;}}
    public bool Advance(TimeSpan elapsed){Authority();if(elapsed<TimeSpan.Zero || elapsed>TimeSpan.FromMinutes(1))throw new InvalidDataException("Invalid cooperative simulation time step.");lock(gate){if(phase!=CoopSessionPhase.Running || elapsed==TimeSpan.Zero)return false;bool changed=false;for(int slot=0;slot<2;slot++)if(health[slot]==0 && health[1-slot]>0 && respawn[slot]>0){respawn[slot]=Math.Max(0,respawn[slot]-elapsed.TotalMilliseconds);if(respawn[slot]==0)health[slot]=identity.Rules.MaxHealth;changed=true;}if(changed)Bump();return changed;}}
    public bool CompleteScenario(){Authority();lock(gate){if(phase!=CoopSessionPhase.Running)return false;phase=CoopSessionPhase.Completed;Bump();return true;}}
    public bool PlayersCanInteract=>identity.Rules.PlayerInteractions;
    public bool EggBlocksTeammate(int sourceSlot,int targetSlot){Slot(sourceSlot);Slot(targetSlot);return false;}
    public bool ApplyTeammateEggDamage(int sourceSlot,int targetSlot,int damage){Authority();Slot(sourceSlot);Slot(targetSlot);if(damage<0)throw new InvalidDataException("Negative egg damage.");return false;}
}
