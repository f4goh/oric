#include "WebUi.h"

static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Oric K7 Emulator</title>
<style>
:root{
  --bg:#0f1419;--panel:#182028;--panel2:#202b36;--border:#2d3b48;
  --text:#e6edf3;--muted:#8b9bb0;--accent:#ff7b29;--accent2:#2ea043;
  --danger:#da3633;--info:#388bfd;
}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;background:var(--bg);color:var(--text);line-height:1.45}
header{background:linear-gradient(90deg,#161b22,#1f2937);border-bottom:2px solid var(--accent);padding:14px 18px;display:flex;flex-wrap:wrap;gap:10px;align-items:center;justify-content:space-between}
.brand{font-weight:700;font-size:1.15rem;letter-spacing:.4px}
.brand span{color:var(--accent)}
.badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:.82rem;background:var(--panel2);border:1px solid var(--border);color:var(--text)}
.badge.ap{border-color:var(--accent);color:#ffb380}
.badge.sta{border-color:var(--accent2);color:#7ee787}
main{max-width:1040px;margin:16px auto;padding:0 14px 40px}
.status-card{background:var(--panel);border:1px solid var(--border);border-radius:10px;padding:12px 16px;margin-bottom:14px}
.status-row{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:8px;font-size:.92rem}
.prog-wrap{width:100%;height:12px;background:#0b0f14;border-radius:999px;overflow:hidden;margin-top:8px;border:1px solid var(--border)}
.prog-bar{height:100%;width:0%;background:linear-gradient(90deg,var(--accent),#ffb347);transition:width .2s ease}
.tabs{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:14px}
.tab-btn{background:var(--panel);color:var(--muted);border:1px solid var(--border);padding:8px 14px;border-radius:8px;cursor:pointer;font-weight:600;font-size:.9rem}
.tab-btn.active{background:var(--accent);color:#111;border-color:var(--accent)}
.tab-panel{display:none;background:var(--panel);border:1px solid var(--border);border-radius:10px;padding:16px}
.tab-panel.active{display:block}
h2{margin:0 0 12px;font-size:1.05rem;color:#fff;border-bottom:1px solid var(--border);padding-bottom:6px}
.row{display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:12px}
input[type=text],input[type=password],select{background:#0d1117;color:var(--text);border:1px solid var(--border);border-radius:6px;padding:8px 10px;font-size:.92rem;min-width:180px}
input[type=file]{color:var(--muted);font-size:.88rem}
button{background:var(--panel2);color:var(--text);border:1px solid var(--border);border-radius:6px;padding:7px 12px;cursor:pointer;font-size:.86rem;font-weight:600}
button:hover{filter:brightness(1.15)}
button.primary{background:var(--accent);color:#111;border-color:var(--accent)}
button.ok{background:var(--accent2);color:#fff;border-color:var(--accent2)}
button.info{background:var(--info);color:#fff;border-color:var(--info)}
button.danger{background:var(--danger);color:#fff;border-color:var(--danger)}
button:disabled{opacity:.45;cursor:not-allowed}
table{width:100%;border-collapse:collapse;font-size:.9rem}
th,td{text-align:left;padding:8px 6px;border-bottom:1px solid var(--border)}
th{color:var(--muted);font-weight:600}
td.actions{display:flex;flex-wrap:wrap;gap:5px}
.form-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:14px;margin-bottom:14px}
.field label{display:block;font-size:.84rem;color:var(--muted);margin-bottom:4px}
.field input,.field select{width:100%}
.check-row{display:flex;align-items:center;gap:8px;margin:8px 0;font-size:.92rem}
.modal-bg{display:none;position:fixed;inset:0;background:rgba(0,0,0,.72);z-index:50;align-items:center;justify-content:center;padding:16px}
.modal-bg.open{display:flex}
.modal{background:var(--panel);border:1px solid var(--border);border-radius:10px;max-width:860px;width:100%;max-height:86vh;display:flex;flex-direction:column}
.modal-head{padding:10px 14px;border-bottom:1px solid var(--border);display:flex;justify-content:space-between;align-items:center}
.modal pre{margin:0;padding:14px;overflow:auto;font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:.84rem;white-space:pre-wrap;color:#d2e3f3;background:#0b0f14;flex:1}
.muted{color:var(--muted);font-size:.85rem}
</style>
</head>
<body>
<header>
  <div class="brand">ORIC <span>K7</span> Emulator</div>
  <div id="netBadge" class="badge">Connexion...</div>
</header>

<main>
  <div class="status-card">
    <div class="status-row">
      <div><strong>Etat K7 :</strong> <span id="k7StateText">Pret</span> <span id="k7File" class="badge" style="margin-left:6px;display:none"></span></div>
      <div id="k7Bytes" class="muted">0 / 0 octets</div>
    </div>
    <div class="prog-wrap"><div id="k7Prog" class="prog-bar"></div></div>
  </div>

  <div class="tabs">
    <button class="tab-btn active" onclick="showTab('files',this)">Cassette &amp; Fichiers</button>
    <button class="tab-btn" onclick="showTab('bmp',this)">Image BMP &rarr; TAP</button>
    <button class="tab-btn" onclick="showTab('cfg',this)">Configuration WiFi &amp; Oric</button>
    <button class="tab-btn" onclick="showTab('help',this)">Aide &amp; Brochage</button>
  </div>

  <section id="tab-files" class="tab-panel active">
    <h2>Gestionnaire SPIFFS &amp; Lecture / Sauvegarde K7</h2>
    <div class="row" style="justify-content:space-between">
      <div id="spiffsInfo" class="muted">SPIFFS : ...</div>
      <div class="row" style="margin:0">
        <input type="file" id="upFile" accept=".tap,.bmp,.txt">
        <button class="primary" onclick="uploadFile()">Envoyer vers Flash</button>
      </div>
    </div>
    <div class="row" style="background:var(--panel2);padding:10px;border-radius:8px;border:1px solid var(--border)">
      <strong>CSAVE (Oric &rarr; ESP32) :</strong>
      <input type="text" id="csaveName" placeholder="nom_fichier.tap">
      <button class="ok" id="btnCsave" onclick="doCsave()">Enregistrer (CSAVE)</button>
      <span id="csaveMsg" class="muted">Lancez CSAVE"..." sur l'Oric</span>
    </div>
    <table>
      <thead><tr><th>Fichier</th><th>Taille</th><th>Actions</th></tr></thead>
      <tbody id="fileRows"><tr><td colspan="3" class="muted">Chargement...</td></tr></tbody>
    </table>
  </section>

  <section id="tab-bmp" class="tab-panel">
    <h2>Convertisseur Image BMP Monochrome (240x200) &rarr; .TAP</h2>
    <p class="muted">Importez d'abord un fichier .bmp monochrome (240x200) dans l'onglet Fichiers, puis convertissez-le en programme HIRES .tap.</p>
    <div class="form-grid">
      <div class="field">
        <label>Fichier source (.bmp en Flash)</label>
        <select id="bmpSrc"></select>
      </div>
      <div class="field">
        <label>Fichier destination (.tap)</label>
        <input type="text" id="bmpDst" placeholder="image.tap">
      </div>
    </div>
    <button class="primary" onclick="convertBmp()">Convertir en .TAP</button>
    <div id="bmpMsg" class="muted" style="margin-top:10px"></div>
  </section>

  <section id="tab-cfg" class="tab-panel">
    <h2>Configuration WiFi, Vitesse &amp; Pont Telnet / ACIA 6551</h2>
    <div class="form-grid">
      <div class="field">
        <label>Mode WiFi</label>
        <select id="cfgInternet">
          <option value="0">Point d'acces (ORIC_AP / 192.168.4.1)</option>
          <option value="1">Client WiFi (Box / Routeur local)</option>
        </select>
      </div>
      <div class="field">
        <label>SSID Box / Routeur</label>
        <input type="text" id="cfgSsid" maxlength="32" placeholder="Nom du reseau WiFi">
      </div>
      <div class="field">
        <label>Mot de passe WiFi (laisser vide pour conserver)</label>
        <input type="password" id="cfgPass" maxlength="64" placeholder="Mot de passe">
      </div>
      <div class="field">
        <label>Vitesse CLOAD</label>
        <select id="cfgSpeed">
          <option value="0">Normal (FAST)</option>
          <option value="1">Rapide (FAST16)</option>
        </select>
      </div>
    </div>
    <div class="check-row">
      <input type="checkbox" id="cfgSerial">
      <label for="cfgSerial">Activer le pont Telnet (port 23) &harr; ACIA 6551 (UART2 GPIO 16/17, 9600 bauds)</label>
    </div>
    <div class="row" style="margin-top:14px">
      <button class="primary" onclick="saveConfig(true)">Enregistrer &amp; Redemarrer</button>
      <button onclick="saveConfig(false)">Enregistrer sans redemarrer</button>
      <button class="danger" onclick="formatSpiffs()">Formater SPIFFS</button>
      <button onclick="rebootEsp()">Redemarrer ESP32</button>
    </div>
    <div id="cfgMsg" class="muted"></div>
  </section>

  <section id="tab-help" class="tab-panel">
    <h2>Utilisation Stand-Alone &amp; Reseau</h2>
    <ul>
      <li><strong>Ecran OLED + Encodeur rotatif :</strong> fonctionne meme sans WiFi pour lancer <code>CLOAD</code> ou <code>CSAVE</code> directement sur la flash interne.</li>
      <li><strong>Mode Point d'Acces (AP) :</strong> reseau <code>ORIC_AP</code> (mot de passe <code>oricatmos</code>), page Web sur <code>http://192.168.4.1</code>.</li>
      <li><strong>Repli automatique :</strong> si la box fibre est hors de portee ou mal configuree, l'ESP32 bascule automatiquement en mode AP <code>ORIC_AP</code> pour rester accessible.</li>
      <li><strong>Forcer le mode AP au demarrage :</strong> maintenez le cavalier/broche <code>GPIO 26</code> a la masse.</li>
      <li><strong>Telnet / ACIA 6551 :</strong> port TCP <code>23</code> relie au port serie UART2 (RX=GPIO16, TX=GPIO17, 9600 bauds).</li>
    </ul>
  </section>
</main>

<div id="viewerBg" class="modal-bg" onclick="if(event.target===this)closeModal()">
  <div class="modal">
    <div class="modal-head">
      <strong id="modalTitle">Apercu</strong>
      <button onclick="closeModal()">Fermer</button>
    </div>
    <pre id="modalBody"></pre>
  </div>
</div>

<script>
function showTab(id,btn){
  document.querySelectorAll('.tab-panel').forEach(p=>p.classList.remove('active'));
  document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
  document.getElementById('tab-'+id).classList.add('active');
  btn.classList.add('active');
}
function closeModal(){document.getElementById('viewerBg').classList.remove('open');}
function openModal(title,text){
  document.getElementById('modalTitle').textContent=title;
  document.getElementById('modalBody').textContent=text||'(vide)';
  document.getElementById('viewerBg').classList.add('open');
}
async function refreshFiles(){
  const r=await fetch('/api/files');
  const files=await r.json();
  const tb=document.getElementById('fileRows');
  const bmpSel=document.getElementById('bmpSrc');
  tb.innerHTML='';
  bmpSel.innerHTML='';
  if(!files.length){
    tb.innerHTML='<tr><td colspan="3" class="muted">Aucun fichier en memoire flash.</td></tr>';
    return;
  }
  files.sort((a,b)=>a.name.localeCompare(b.name));
  for(const f of files){
    const tr=document.createElement('tr');
    const isTap=f.name.toLowerCase().endsWith('.tap');
    const isBmp=f.name.toLowerCase().endsWith('.bmp');
    if(isBmp){
      const o=document.createElement('option');
      o.value=f.name;o.textContent=f.name;
      bmpSel.appendChild(o);
    }
    const enc=encodeURIComponent(f.name);
    tr.innerHTML=`<td><strong>${esc(f.name)}</strong></td><td>${f.size} o</td><td class="actions">`+
      (isTap?`<button class="primary" onclick="doCload('${enc}')">CLOAD</button>`:'')+
      (isTap?`<button class="info" onclick="viewText('/api/basic?file=${enc}','BASIC : ${esc(f.name)}')">BASIC</button>`:'')+
      (isTap?`<button onclick="doDisasm('${enc}')">ASM</button>`:'')+
      `<button onclick="viewText('/api/hex?file=${enc}','HEX : ${esc(f.name)}')">HEX</button>`+
      `<a href="/download?file=${enc}"><button type="button">DL</button></a>`+
      `<button class="danger" onclick="delFile('${enc}')">Suppr</button></td>`;
    tb.appendChild(tr);
  }
}
function esc(s){return s.replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));}
async function viewText(url,title){
  openModal(title,'Chargement...');
  const r=await fetch(url);
  document.getElementById('modalBody').textContent=await r.text();
}
async function doDisasm(enc){
  const name=decodeURIComponent(enc);
  const saveTxt=confirm('Voulez-vous aussi sauvegarder le desassemblage en .txt dans la flash ?');
  let url='/api/disasm?file='+enc;
  if(saveTxt){
    const dst=name.replace(/\.tap$/i,'')+'.txt';
    url+='&save='+encodeURIComponent(dst);
  }
  await viewText(url,'ASM 6502 : '+name);
  if(saveTxt)refreshFiles();
}
let cloadLock=false;
async function doCload(enc){
  if(cloadLock)return;
  cloadLock=true;
  const name=decodeURIComponent(enc);
  document.getElementById('k7StateText').textContent='Attente CLOAD sur l\'Oric...';
  const kf=document.getElementById('k7File');
  kf.textContent=name;kf.style.display='inline-block';
  document.getElementById('k7Prog').style.width='0%';
  try{
    const r=await fetch('/api/cload?file='+enc,{method:'POST'});
    if(!r.ok)alert(await r.text());
  }finally{
    cloadLock=false;
    pollStatus();
  }
}
let csaveLock=false;
async function doCsave(){
  if(csaveLock)return;
  let name=document.getElementById('csaveName').value.trim();
  if(!name){alert('Entrez un nom de fichier (ex: prog.tap)');return;}
  if(!name.toLowerCase().endsWith('.tap'))name+='.tap';
  csaveLock=true;
  document.getElementById('csaveMsg').textContent='Attente CSAVE sur l\'Oric : '+name+'...';
  const kf=document.getElementById('k7File');
  kf.textContent=name;kf.style.display='inline-block';
  try{
    const r=await fetch('/api/csave?file='+encodeURIComponent(name),{method:'POST'});
    if(!r.ok)alert(await r.text());
  }finally{
    csaveLock=false;
    pollStatus();
  }
}
async function delFile(enc){
  if(!confirm('Supprimer '+decodeURIComponent(enc)+' ?'))return;
  await fetch('/api/delete?file='+enc,{method:'POST'});
  refreshFiles();pollStatus();
}
async function uploadFile(){
  const inp=document.getElementById('upFile');
  if(!inp.files.length){alert('Choisissez un fichier');return;}
  const fd=new FormData();
  fd.append('file',inp.files[0]);
  const r=await fetch('/upload',{method:'POST',body:fd});
  if(!r.ok)alert('Erreur upload: '+await r.text());
  inp.value='';
  refreshFiles();pollStatus();
}
async function convertBmp(){
  const src=document.getElementById('bmpSrc').value;
  let dst=document.getElementById('bmpDst').value.trim();
  if(!src||!dst){document.getElementById('bmpMsg').textContent='Selectionnez un .bmp et entrez un nom .tap.';return;}
  if(!dst.toLowerCase().endsWith('.tap'))dst+='.tap';
  const r=await fetch('/api/convert?src='+encodeURIComponent(src)+'&dst='+encodeURIComponent(dst),{method:'POST'});
  const j=await r.json();
  document.getElementById('bmpMsg').textContent=j.ok?'Conversion reussie : '+dst:'Erreur conversion';
  refreshFiles();
}
async function loadConfig(){
  const r=await fetch('/api/config');
  const c=await r.json();
  document.getElementById('cfgInternet').value=c.internet?'1':'0';
  document.getElementById('cfgSsid').value=c.ssid||'';
  document.getElementById('cfgSpeed').value=c.speed?'1':'0';
  document.getElementById('cfgSerial').checked=!!c.serial;
}
async function saveConfig(reboot){
  const p=new URLSearchParams();
  p.set('internet',document.getElementById('cfgInternet').value);
  p.set('ssid',document.getElementById('cfgSsid').value.trim());
  const pw=document.getElementById('cfgPass').value;
  if(pw.length>0)p.set('pass',pw);
  p.set('speed',document.getElementById('cfgSpeed').value);
  p.set('serial',document.getElementById('cfgSerial').checked?'1':'0');
  p.set('reboot',reboot?'1':'0');
  const r=await fetch('/api/config',{method:'POST',body:p});
  document.getElementById('cfgMsg').textContent=await r.text();
}
async function formatSpiffs(){
  if(!confirm('Effacer TOUS les fichiers .tap en flash ? (La page Web reste intacte)'))return;
  const r=await fetch('/api/format',{method:'POST'});
  alert(await r.text());
  refreshFiles();pollStatus();
}
async function rebootEsp(){
  if(!confirm('Redemarrer l\'ESP32 ?'))return;
  await fetch('/api/reboot',{method:'POST'});
  alert('Redemarrage en cours...');
}
let prevState=-1,loadStartMs=0,estTotal=0,estSpeed16=false,loadingActive=false,pollInFlight=false;
function animateLoading(){
  if(!loadingActive||estTotal<=0)return;
  const bps=estSpeed16?212:150;
  const overhead=270; /* 256 sync (0x16) + 0x24 + header + nom + gap */
  const effTotal=estTotal+overhead;
  const elapsedBytes=Math.round(((Date.now()-loadStartMs)/1000)*bps);
  const pct=Math.min(98,Math.round(Math.min(effTotal,elapsedBytes)*100/effTotal));
  const shown=Math.min(estTotal,Math.max(0,elapsedBytes-overhead));
  document.getElementById('k7Prog').style.width=pct+'%';
  document.getElementById('k7Bytes').textContent=(elapsedBytes<overhead?'Preambule... ':shown+' / '+estTotal+' octets ')+' (~'+pct+'%)';
}
async function pollStatus(){
  if(pollInFlight)return;
  pollInFlight=true;
  const ctrl=new AbortController();
  const tid=setTimeout(()=>ctrl.abort(),900);
  try{
    const r=await fetch('/api/status',{signal:ctrl.signal});
    const s=await r.json();
    const nb=document.getElementById('netBadge');
    nb.textContent=(s.mode==='AP'?'AP : ':'WiFi : ')+s.ip;
    nb.className='badge '+(s.mode==='AP'?'ap':'sta');
    document.getElementById('k7StateText').textContent=s.stateText;
    const kf=document.getElementById('k7File');
    if(s.file){kf.textContent=s.file;kf.style.display='inline-block';}
    else{kf.style.display='none';}
    const csMsg=document.getElementById('csaveMsg');
    if(s.state===3)csMsg.textContent='Attente CSAVE sur l\'Oric ('+s.file+')...';
    else if(s.state===4)csMsg.textContent='Enregistrement CSAVE en cours ('+s.file+')...';
    else if(s.state===5&&s.lastSize>0&&prevState===4||s.state===5&&s.stateText.includes('CSAVE'))csMsg.textContent='Sauvegarde reussie : '+s.file+' ('+s.lastSize+' octets)';
    else if(s.state===6&&prevState===3)csMsg.textContent='Timeout CSAVE (Remote non detecte)';
    if(s.state===2){
      if(!loadingActive){loadStartMs=Date.now();loadingActive=true;}
      estTotal=s.total;estSpeed16=!!s.speed16;
      animateLoading();
    }else{
      loadingActive=false;loadStartMs=0;
      const pct=s.total>0?Math.min(100,Math.round(s.progress*100/s.total)):(s.state===5?100:0);
      document.getElementById('k7Bytes').textContent=s.total>0?(s.progress+' / '+s.total+' octets'):(s.progress>0?(s.progress+' octets'):'');
      document.getElementById('k7Prog').style.width=pct+'%';
    }
    const usedK=Math.round(s.spiffsUsed/1024),totK=Math.round(s.spiffsTotal/1024);
    document.getElementById('spiffsInfo').textContent=`Flash SPIFFS : ${usedK} Ko / ${totK} Ko utilises`;
    if(prevState!==5&&s.state===5){
      if(s.lastSize>0&&csMsg.textContent.includes('CSAVE'))csMsg.textContent='Sauvegarde reussie : '+s.file+' ('+s.lastSize+' octets)';
      refreshFiles();
    }
    prevState=s.state;
  }catch(e){}finally{
    clearTimeout(tid);
    pollInFlight=false;
  }
}
refreshFiles();loadConfig();pollStatus();
setInterval(pollStatus,300);
setInterval(animateLoading,150);
</script>
</body>
</html>)rawliteral";

WebUi::WebUi(Oric *oric) :
    loric(oric),
    server(80),
    taskHandle(nullptr),
    isApMode(true),
    pendingAction(WEB_ACT_NONE),
    rebootPending(false),
    rebootAtMs(0),
    uploadError(false)
{
}

String WebUi::cleanName(String name)
{
    name.trim();
    while (name.startsWith("/")) {
        name.remove(0, 1);
    }
    name.replace("..", "");
    return name;
}

String WebUi::jsonEscape(const String &s)
{
    String out;
    out.reserve(s.length() + 8);
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if ((uint8_t)c >= 0x20) {
            out += c;
        }
    }
    return out;
}

const char *WebUi::stateToText(K7State st)
{
    switch (st) {
        case K7_IDLE:             return "Pret";
        case K7_WAIT_REMOTE_LOAD: return "Attente CLOAD sur l'Oric...";
        case K7_LOADING:          return "Transfert CLOAD en cours...";
        case K7_WAIT_REMOTE_SAVE: return "Attente CSAVE sur l'Oric...";
        case K7_SAVING:           return "Enregistrement CSAVE...";
        case K7_DONE:             return "Termine avec succes";
        case K7_TIMEOUT:          return "Timeout (Remote Oric non detecte)";
        case K7_ERROR:            return "Erreur K7 / Fichier";
        default:                  return "Inconnu";
    }
}

void WebUi::setNetworkInfo(bool apMode, const String &ipStr)
{
    isApMode  = apMode;
    ipAddress = ipStr;
}

bool WebUi::takePendingAction(WebPendingAction &act, String &fileName)
{
    if (pendingAction == WEB_ACT_NONE) {
        return false;
    }
    act = pendingAction;
    fileName = pendingFile;
    pendingAction = WEB_ACT_NONE;
    return true;
}

void WebUi::begin(bool apMode, const String &ipStr)
{
    setNetworkInfo(apMode, ipStr);

    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/files", HTTP_GET, [this]() { handleFiles(); });
    server.on("/upload", HTTP_POST,
              [this]() { handleUploadDone(); },
              [this]() { handleUploadData(); });
    server.on("/download", HTTP_GET, [this]() { handleDownload(); });
    server.on("/api/delete", HTTP_POST, [this]() { handleDelete(); });
    server.on("/api/cload", HTTP_POST, [this]() { handleCload(); });
    server.on("/api/csave", HTTP_POST, [this]() { handleCsave(); });
    server.on("/api/basic", HTTP_GET, [this]() { handleBasic(); });
    server.on("/api/hex", HTTP_GET, [this]() { handleHex(); });
    server.on("/api/disasm", HTTP_GET, [this]() { handleDisasm(); });
    server.on("/api/convert", HTTP_POST, [this]() { handleConvert(); });
    server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
    server.on("/api/config", HTTP_POST, [this]() { handlePostConfig(); });
    server.on("/api/format", HTTP_POST, [this]() { handleFormat(); });
    server.on("/api/reboot", HTTP_POST, [this]() { handleReboot(); });

    server.begin();
    Serial.println("HTTP WebServer started on port 80");
}

void WebUi::run()
{
    handleClientLoop();
}

void WebUi::webTask(void *param)
{
    WebUi *self = static_cast<WebUi *>(param);
    for (;;) {
        self->handleClientLoop();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void WebUi::handleClientLoop()
{
    server.handleClient();
    if (rebootPending && (int32_t)(millis() - rebootAtMs) >= 0) {
        ESP.restart();
    }
}

void WebUi::handleRoot()
{
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void WebUi::handleStatus()
{
    K7State st = loric->getK7State();
    String json = "{";
    json += "\"mode\":\"" + String(isApMode ? "AP" : "STA") + "\",";
    json += "\"ip\":\"" + jsonEscape(ipAddress) + "\",";
    json += "\"state\":" + String((int)st) + ",";
    json += "\"stateText\":\"" + String(stateToText(st)) + "\",";
    json += "\"file\":\"" + jsonEscape(loric->getK7FileName()) + "\",";
    json += "\"progress\":" + String(loric->getK7ProgressBytes()) + ",";
    json += "\"total\":" + String(loric->getK7TotalBytes()) + ",";
    json += "\"lastSize\":" + String(loric->getK7LastSize()) + ",";
    json += "\"speed16\":" + String(loric->getSpeed() == FAST16 ? "true" : "false") + ",";
    json += "\"spiffsTotal\":" + String((uint32_t)SPIFFS.totalBytes()) + ",";
    json += "\"spiffsUsed\":" + String((uint32_t)SPIFFS.usedBytes());
    json += "}";
    server.send(200, "application/json", json);
}

void WebUi::handleFiles()
{
    String json = "[";
    File root = SPIFFS.open("/");
    bool first = true;
    if (root && root.isDirectory()) {
        File f = root.openNextFile();
        while (f) {
            String name = cleanName(String(f.name()));
            if (name.length() > 0) {
                if (!first) json += ",";
                first = false;
                json += "{\"name\":\"" + jsonEscape(name) + "\",\"size\":" + String((uint32_t)f.size()) + "}";
            }
            f = root.openNextFile();
        }
    }
    json += "]";
    server.send(200, "application/json", json);
}

void WebUi::handleUploadData()
{
    HTTPUpload &up = server.upload();
    if (up.status == UPLOAD_FILE_START) {
        uploadError = false;
        K7State st = loric->getK7State();
        if (st == K7_LOADING || st == K7_SAVING || st == K7_WAIT_REMOTE_LOAD || st == K7_WAIT_REMOTE_SAVE) {
            uploadError = true;
            return;
        }
        String fn = cleanName(up.filename);
        if (fn.length() == 0) {
            uploadError = true;
            return;
        }
        uploadFile = SPIFFS.open("/" + fn, "w");
        if (!uploadFile) {
            uploadError = true;
        }
    } else if (up.status == UPLOAD_FILE_WRITE) {
        if (!uploadError && uploadFile) {
            if (uploadFile.write(up.buf, up.currentSize) != up.currentSize) {
                uploadError = true;
            }
        }
    } else if (up.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
        }
    }
}

void WebUi::handleUploadDone()
{
    if (uploadError) {
        server.send(500, "text/plain", "Upload impossible (transfert K7 actif ou erreur SPIFFS)");
    } else {
        server.send(200, "text/plain", "OK");
    }
}

void WebUi::handleDownload()
{
    String fn = cleanName(server.arg("file"));
    if (fn.length() == 0 || !SPIFFS.exists("/" + fn)) {
        server.send(404, "text/plain", "Fichier introuvable");
        return;
    }
    File f = SPIFFS.open("/" + fn, "r");
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + fn + "\"");
    server.streamFile(f, "application/octet-stream");
    f.close();
}

void WebUi::handleDelete()
{
    K7State st = loric->getK7State();
    if (st == K7_LOADING || st == K7_SAVING) {
        server.send(409, "text/plain", "Transfert K7 en cours");
        return;
    }
    String fn = cleanName(server.arg("file"));
    if (fn.length() > 0 && SPIFFS.exists("/" + fn)) {
        SPIFFS.remove("/" + fn);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(404, "text/plain", "Fichier introuvable");
    }
}

void WebUi::handleCload()
{
    K7State st = loric->getK7State();
    if (st == K7_LOADING || st == K7_SAVING || st == K7_WAIT_REMOTE_SAVE) {
        server.send(409, "text/plain", "Une operation K7 est deja en cours");
        return;
    }
    String fn = cleanName(server.arg("file"));
    if (fn.length() == 0 || !SPIFFS.exists("/" + fn)) {
        server.send(404, "text/plain", "Fichier .tap introuvable");
        return;
    }
    if (st == K7_WAIT_REMOTE_LOAD) {
        if (fn == loric->getK7FileName() || (pendingAction == WEB_ACT_CLOAD && fn == pendingFile)) {
            server.send(200, "text/plain", "Fichier deja selectionne");
            return;
        }
        loric->requestAbort();
        pendingFile   = fn;
        pendingAction = WEB_ACT_CLOAD;
        server.send(200, "text/plain", "Fichier CLOAD remplace");
        return;
    }
    if (pendingAction == WEB_ACT_CLOAD && fn == pendingFile) {
        server.send(200, "text/plain", "Fichier deja selectionne");
        return;
    }
    pendingFile   = fn;
    pendingAction = WEB_ACT_CLOAD;
    server.send(200, "text/plain", "CLOAD lance");
}

void WebUi::handleCsave()
{
    K7State st = loric->getK7State();
    if (st == K7_LOADING || st == K7_SAVING || st == K7_WAIT_REMOTE_LOAD) {
        server.send(409, "text/plain", "Une operation K7 est deja en cours");
        return;
    }
    String fn = cleanName(server.arg("file"));
    if (fn.length() == 0) {
        server.send(400, "text/plain", "Nom de fichier manquant");
        return;
    }
    if (st == K7_WAIT_REMOTE_SAVE) {
        if (fn == loric->getK7FileName() || (pendingAction == WEB_ACT_CSAVE && fn == pendingFile)) {
            server.send(200, "text/plain", "CSAVE deja en attente");
            return;
        }
        loric->requestAbort();
        pendingFile   = fn;
        pendingAction = WEB_ACT_CSAVE;
        server.send(200, "text/plain", "Nom CSAVE remplace");
        return;
    }
    if (pendingAction == WEB_ACT_CSAVE && fn == pendingFile) {
        server.send(200, "text/plain", "CSAVE deja en attente");
        return;
    }
    pendingFile   = fn;
    pendingAction = WEB_ACT_CSAVE;
    server.send(200, "text/plain", "CSAVE lance");
}

void WebUi::handleBasic()
{
    String fn = cleanName(server.arg("file"));
    String out = loric->tap2BasToString(fn);
    server.send(200, "text/plain; charset=utf-8", out);
}

void WebUi::handleHex()
{
    String fn = cleanName(server.arg("file"));
    String out = loric->viewFileToString(fn, 16384);
    server.send(200, "text/plain; charset=utf-8", out);
}

void WebUi::handleDisasm()
{
    String fn  = cleanName(server.arg("file"));
    String dst = cleanName(server.arg("save"));
    String out = loric->desassembleToString(fn, dst);
    server.send(200, "text/plain; charset=utf-8", out);
}

void WebUi::handleConvert()
{
    String src = cleanName(server.arg("src"));
    String dst = cleanName(server.arg("dst"));
    if (src.length() == 0 || dst.length() == 0 || !SPIFFS.exists("/" + src)) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }
    loric->conversion(src, dst);
    bool ok = SPIFFS.exists("/" + dst);
    server.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

void WebUi::handleGetConfig()
{
    Preferences prefs;
    prefs.begin("oric", true);
    bool   internet = prefs.getBool("internet", false);
    String ssid     = prefs.getString("ssid", "");
    bool   speed    = prefs.getBool("speed", false);
    bool   serial   = prefs.getBool("serial", true);
    prefs.end();

    String json = "{";
    json += "\"internet\":" + String(internet ? "true" : "false") + ",";
    json += "\"ssid\":\"" + jsonEscape(ssid) + "\",";
    json += "\"speed\":" + String(speed ? "true" : "false") + ",";
    json += "\"serial\":" + String(serial ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
}

void WebUi::handlePostConfig()
{
    Preferences prefs;
    prefs.begin("oric", false);
    if (server.hasArg("internet")) {
        prefs.putBool("internet", server.arg("internet") == "1");
    }
    if (server.hasArg("ssid")) {
        prefs.putString("ssid", server.arg("ssid"));
    }
    if (server.hasArg("pass") && server.arg("pass").length() > 0) {
        prefs.putString("pass", server.arg("pass"));
    }
    if (server.hasArg("speed")) {
        bool sp = (server.arg("speed") == "1");
        prefs.putBool("speed", sp);
        loric->setSpeed(sp ? FAST16 : FAST);
    }
    if (server.hasArg("serial")) {
        prefs.putBool("serial", server.arg("serial") == "1");
    }
    prefs.putBool("config", true);
    prefs.end();

    if (server.hasArg("reboot") && server.arg("reboot") == "1") {
        rebootPending = true;
        rebootAtMs    = millis() + 400;
        server.send(200, "text/plain", "Configuration enregistree. Redemarrage...");
    } else {
        server.send(200, "text/plain", "Configuration enregistree.");
    }
}

void WebUi::handleFormat()
{
    K7State st = loric->getK7State();
    if (st == K7_LOADING || st == K7_SAVING) {
        server.send(409, "text/plain", "Transfert K7 en cours");
        return;
    }
    bool ok = SPIFFS.format();
    server.send(ok ? 200 : 500, "text/plain", ok ? "SPIFFS formate avec succes." : "Erreur formatage SPIFFS.");
}

void WebUi::handleReboot()
{
    rebootPending = true;
    rebootAtMs    = millis() + 400;
    server.send(200, "text/plain", "Redemarrage...");
}
