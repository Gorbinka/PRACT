const data = {
    was: [{l:"Вес",v:"85кг"},{l:"Рост",v:"175см"},{l:"Грудь",v:"95см"},{l:"Талия",v:"90см"},{l:"Бедра",v:"85см"},{l:"Жир",v:"30%"}],
    now: [{l:"Вес",v:"70кг"},{l:"Рост",v:"175см"},{l:"Грудь",v:"85см"},{l:"Талия",v:"75см"},{l:"Бедра",v:"75см"},{l:"Жир",v:"20%"}]
};

// Переключение экранов (Прогресс / Чат)
const navChat = document.getElementById('nav-chat');
const navProgress = document.getElementById('nav-progress');
const screenChat = document.getElementById('screen-chat');
const screenProgress = document.getElementById('screen-progress');

navChat.onclick = () => {
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    navChat.classList.add('active');
    screenProgress.classList.remove('active');
    screenChat.classList.add('active');
};

navProgress.onclick = () => {
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    navProgress.classList.add('active');
    screenChat.classList.remove('active');
    screenProgress.classList.add('active');
};

// Логика Прогресса (Было / Стало / Цель)
const statsContainer = document.getElementById('stats-container');
const viewStats = document.getElementById('view-stats');
const viewGoal = document.getElementById('view-goal');

function renderStats(type) {
    statsContainer.innerHTML = data[type].map(item => {
        const isInt = ["Грудь","Талия","Бедра"].includes(item.l);
        return `<div class="stat-item ${isInt?'int':''}" onclick="selZone(this, '${item.l}')"><span class="dot"></span>${item.l}: ${item.v}</div>`;
    }).join('');
}

function selZone(el, label) {
    if (!["Грудь","Талия","Бедра"].includes(label)) return;
    const id = label==="Грудь"?"zone-chest":label==="Талия"?"zone-waist":"zone-hips";
    const zone = document.getElementById(id);
    const wasAct = el.classList.contains('active');
    document.querySelectorAll('.stat-item, .body-zone').forEach(x => x.classList.remove('active'));
    if (!wasAct) { el.classList.add('active'); zone.classList.add('active'); }
}

document.querySelectorAll('.toggle-btn').forEach(btn => {
    btn.onclick = () => {
        document.querySelectorAll('.toggle-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        if (btn.id === 'btn-goal') {
            viewStats.classList.remove('active'); viewGoal.classList.add('active');
            updateGoal(28); // 28% - закраска второго до бедер
        } else {
            viewGoal.classList.remove('active'); viewStats.classList.add('active');
            renderStats(btn.id === 'btn-was' ? 'was' : 'now');
        }
    };
});

// Закрашивание уровней в "Цели"
function updateGoal(pct) {
    const progressFill = document.querySelector('.progress-fill');
    if (progressFill) { progressFill.style.width = '35%'; progressFill.innerText = '35%'; }

    for (let i = 1; i <= 5; i++) {
        const maskRect = document.getElementById(`fill-${i}`);
        if (!maskRect) continue;
        let lp = Math.min(100, Math.max(0, (pct - (i-1)*20) * 5));
        maskRect.setAttribute('y', 200 - (lp * 2));
    }
}

// Чат
const chatInput = document.getElementById('chat-input');
const chatMessages = document.getElementById('chat-messages');
document.getElementById('send-btn').onclick = () => {
    if (!chatInput.value.trim()) return;
    chatMessages.innerHTML += `<div class="message user"><div class="message-bubble">${chatInput.value}</div></div>`;
    chatInput.value = "";
    chatMessages.scrollTop = chatMessages.scrollHeight;
};

// Старт
renderStats('now');
