// Exercise the production Linux type-change event and bubble predicate directly.
// DOM/API/undo are explicit boundaries; no browser, server or native game runs.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file=process.argv[4]||path.join(root,'pc/workshop/WorkshopLinux/index.html');
const html=fs.readFileSync(file,'utf8'),script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];assert.ok(script,'Production inline script absent');
const lines=script.split(/\r?\n/),single=prefix=>{const result=lines.filter(v=>v.startsWith(prefix));assert.equal(result.length,1,prefix);return result[0]};
const actual={helper:single('function isFruitBubble('),editable:single('function editable('),run:single('function run('),handler:single("$('type').onchange=")};
let checks=0,cases=0;const check=(v,msg)=>{checks++;assert.ok(v,msg)},same=(a,b,msg)=>{checks++;assert.deepEqual(a,b,msg)},clone=v=>v==null?v:JSON.parse(JSON.stringify(v));
const native=[...Array.from({length:5},(_,i)=>0x41d1+i),...Array.from({length:5},(_,i)=>0x468d+i),...Array.from({length:5},(_,i)=>0x452a+i),0x46a8,0x46a9];
const hybrid=Array.from({length:6},(_,i)=>0x4f20+i),bubbles=[...native,...hybrid],loose=[...Array.from({length:5},(_,i)=>0x4030+i),...Array.from({length:6},(_,i)=>0x4f00+i)];
const oldHandler="$('type').onchange=run(async()=>{if(!editable(selected))return;const next=await api('placement/'+$('type').value+'?room='+activeRoom),bubble=id=>(id>=0x4f20&&id<=0x4f25)||(id>=0x41d1&&id<=0x41d5)||(id>=0x468d&&id<=0x4691)||(id>=0x452a&&id<=0x452e);mark();if(!bubble(selected.ObjectId)||!bubble(next.ObjectId))selected.Layer=next.Layer;selected.ObjectId=next.ObjectId;selected.HybridVariant=next.HybridVariant??null;fields()});";
function setup(source,target,{old=false,error=false,layer=0x1234,templateLayer=0x24}={}){
 const type={value:String(target)},original=source==null?null:{Id:'4'.repeat(32),ObjectId:source,Layer:layer,X:123.5,Y:-42.25,HybridVariant:5};let requests=[],history=[],fieldCalls=0,errors=[];
 const context=vm.createContext({selected:clone(original),activeRoom:81,$:id=>{assert.equal(id,'type');return type},api:async route=>{requests.push(route);if(error)throw Error('Synthetic API refusal');return {Id:'9'.repeat(32),ObjectId:target,Layer:templateLayer,X:999,Y:999,HybridVariant:null}},mark:()=>history.push(clone(context.selected)),fields:()=>fieldCalls++,fail:e=>errors.push(e.message),console});
 new vm.Script([actual.helper,actual.editable,actual.run,old?oldHandler:actual.handler].join('\n'),{filename:file}).runInContext(context);
 return {context,type,original,requests,history,errors,get fieldCalls(){return fieldCalls}};
}
async function replace(source,target,preserve,options={}){
 const t=setup(source,target,options);await t.type.onchange();cases++;
 same(t.errors,[],'Unexpected event failure');same(t.requests,['placement/'+target+'?room=81'],'Current room API routing');same(t.history,[t.original],'Undo captures exact previous placement');same(t.fieldCalls,1,'Fields refresh once');
 same(t.context.selected.Id,t.original.Id,'Identity retained');same(t.context.selected.X,t.original.X,'X retained');same(t.context.selected.Y,t.original.Y,'Y retained');same(t.context.selected.ObjectId,target,'Target kind selected');same(t.context.selected.HybridVariant,null,'No stale Maskass hybrid variant');same(t.context.selected.Layer,preserve?t.original.Layer:(options.templateLayer??0x24),'Native layer policy');
}
(async()=>{
 const classification=setup(0x4030,0x4030),known=new Set(bubbles);
 for(let id=0;id<65536;id++)check(classification.context.isFruitBubble(id)===known.has(id),'Bubble classification mismatch:'+id.toString(16));
 for(const source of bubbles)for(const target of bubbles)await replace(source,target,true);
 for(const source of loose)for(const target of loose)await replace(source,target,false);
 for(const source of [0x46a8,0x46a9,0x4f20])for(const target of [0x4030,0x4f05]){await replace(source,target,false);await replace(target,source,false)}
 for(const source of [null,0x4001,0x8000]){const t=setup(source,0x4f20);await t.type.onchange();same(t.requests,[],'Protected selection never calls API');same(t.history,[],'Protected selection has no undo');same(clone(t.context.selected),t.original,'Protected selection unchanged')}
 const failure=setup(0x46a8,0x4f22,{error:true});await failure.type.onchange();same(failure.errors,['Synthetic API refusal'],'API refusal remains visible');same(failure.history,[],'API failure has no undo');same(clone(failure.context.selected),failure.original,'API failure preserves exact original');same(failure.fieldCalls,0,'API failure no fake refresh');
 for(const source of [0x46a8,0x46a9]){const t=setup(source,0x4f20,{old:true});await t.type.onchange();check(t.context.selected.Layer!==t.original.Layer,'Before-fix control must lose alias native layer')}
 let actualNativeLossWitnessed=0,nativeAliasPairs=0;
 if(process.argv[3]){const core=JSON.parse(fs.readFileSync(process.argv[3],'utf8'));check(core.passed===true&&core.originalRomUnchanged===true,'Core native probe must pass');same([...new Set(core.coreBubbleKinds)].sort((a,b)=>a-b),[...known].sort((a,b)=>a-b),'Actual C# Core and Linux predicate agree');
  for(const row of core.nativeAliasPlacements){const source=parseInt(row.id,16),options={layer:row.layer,templateLayer:row.templateLayer};for(const target of hybrid){await replace(source,target,true,options);nativeAliasPairs++;}if(row.wouldLoseLayerUnderOldHandler){const old=setup(source,0x4f20,{...options,old:true});await old.type.onchange();check(old.context.selected.Layer!==old.original.Layer,'Actual native alias placement witnesses previous loss');actualNativeLossWitnessed++;}}
  same(actualNativeLossWitnessed,core.placementsWithDifferentTemplateLayer,'All actual native loss witnesses checked');
 }
 const report={passed:true,checks,replacementCases:cases,nativeAliasPairs,actualNativeLossWitnessed,bubbleKinds:bubbles.length,bubblePairs:bubbles.length**2,loosePairs:loose.length**2,oldNativeAliasLayerLossReproduced:2,actualProductionHandler:true,actualProductionHelper:true,apiBoundaryExplicit:true,coreProbeCompared:!!process.argv[3],sourceSha256:crypto.createHash('sha256').update(html).digest('hex'),handlerSha256:crypto.createHash('sha256').update(actual.handler).digest('hex'),browserGameGpuOrPersonalProfileUsed:false};
 if(process.argv[2])fs.writeFileSync(process.argv[2],JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2));
})().catch(e=>{console.error('FAIL',e);process.exitCode=1});
