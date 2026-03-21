const data = {
    was: [
        { label: "Вес", value: "85кг", color: "green" },
        { label: "Рост", value: "175см", color: "green" },
        { label: "Грудь", value: "95см", color: "green" },
        { label: "Талия", value: "90см", color: "pink" },
        { label: "Бедра", value: "85см", color: "green" },
        { label: "Жир", value: "30%", color: "green" }
    ],
    now: [
        { label: "Вес", value: "70кг", color: "green" },
        { label: "Рост", value: "175см", color: "green" },
        { label: "Грудь", value: "85см", color: "green" },
        { label: "Талия", value: "75см", color: "pink" },
        { label: "Бедра", value: "75см", color: "green" },
        { label: "Жир", value: "20%", color: "green" }
    ]
};

const statsContainer = document.getElementById('stats-container');
const buttons = document.querySelectorAll('.toggle-btn');
const viewStats = document.getElementById('view-stats');
const viewGoal = document.getElementById('view-goal');

function renderStats(state) {
    const items = data[state];
    if (!items) return;
    
    statsContainer.innerHTML = items.map(item => {
        const zoneId = getZoneId(item.label);
        // Добавляем класс interactive только для нужных параметров
        const isInteractive = zoneId !== null;
        
        return `
            <div class="stat-item ${isInteractive ? 'interactive' : ''}" 
                 data-zone="${zoneId || ''}" 
                 onclick="${isInteractive ? 'selectZone(this)' : ''}">
                <span class="dot"></span> 
                ${item.label}: ${item.value}
            </div>
        `;
    }).join('');
}

function getZoneId(label) {
    if (label.includes("Грудь")) return "zone-chest";
    if (label.includes("Талия")) return "zone-waist";
    if (label.includes("Бедра")) return "zone-hips";
    return null; // Остальные параметры не интерактивны
}

function selectZone(element) {
    const zoneId = element.getAttribute('data-zone');
    const zoneElement = document.getElementById(zoneId);
    
    // Проверяем, нажат ли уже этот элемент
    const isAlreadyActive = element.classList.contains('active');

    // 1. Сначала в любом случае сбрасываем ВСЕ активные классы
    document.querySelectorAll('.stat-item').forEach(item => item.classList.remove('active'));
    document.querySelectorAll('.body-zone').forEach(zone => zone.classList.remove('active'));

    // 2. Если элемент НЕ был активен — активируем его
    // Если БЫЛ активен — мы его уже сбросили шагом выше (эффект выключения)
    if (!isAlreadyActive) {
        element.classList.add('active');
        if (zoneElement) {
            zoneElement.classList.add('active');
        }
    }
}

function updateGoalProgress(totalPercent) {
    // Обновляем текстовое значение в баре
    const progressFill = document.querySelector('.progress-fill');
    if (progressFill) {
        progressFill.style.width = totalPercent + '%';
        progressFill.innerText = totalPercent + '%';
    }

    // Распределяем проценты по 5 человечкам (каждый по 20%)
    for (let i = 1; i <= 5; i++) {
        const maskRect = document.getElementById(`fill-${i}`);
        if (!maskRect) continue;

        let levelPercent = 0;
        const minRange = (i - 1) * 20;
        const maxRange = i * 20;

        if (totalPercent >= maxRange) {
            levelPercent = 100; // Полностью закрашен
        } else if (totalPercent > minRange) {
            // Частично закрашен: вычисляем долю внутри этих 20%
            levelPercent = ((totalPercent - minRange) / 20) * 100;
        } else {
            levelPercent = 0; // Пустой
        }

        // В SVG маске y=200 - пусто, y=0 - полно. 
        // Инвертируем процент в координату Y
        const yValue = 200 - (levelPercent * 2); 
        maskRect.setAttribute('y', yValue);
    }
}

// Вызовите эту функцию при клике на кнопку "Цель"
// Например, внутри обработчика клика:
// if (btn.id === 'btn-goal') updateGoalProgress(35);


// Обновите вызов функции
updateGoalProgress(35); 

// И в самой функции script.js измените расчет для масок:
function updateGoalProgress(totalPercent) {
    const progressFill = document.querySelector('.progress-fill');
    if (progressFill) {
        progressFill.style.width = totalPercent + '%';
        progressFill.innerText = totalPercent + '%';
    }

    // Используем визуальный коэффициент, чтобы закрашивалось чуть меньше
    const visualPercent = totalPercent * 0.8; // Уменьшаем визуальную закраску

    for (let i = 1; i <= 5; i++) {
        const maskRect = document.getElementById(`fill-${i}`);
        if (!maskRect) continue;

        let levelPercent = 0;
        const minRange = (i - 1) * 20;
        const maxRange = i * 20;

        if (visualPercent >= maxRange) {
            levelPercent = 100;
        } else if (visualPercent > minRange) {
            levelPercent = ((visualPercent - minRange) / 20) * 100;
        } else {
            levelPercent = 0;
        }

        const yValue = 200 - (levelPercent * 2); 
        maskRect.setAttribute('y', yValue);
    }
}






buttons.forEach(btn => {
    btn.addEventListener('click', () => {
        buttons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        if (btn.id === 'btn-goal') {
            // Показываем вид "ЦЕЛЬ"
            viewStats.classList.remove('active');
            viewGoal.classList.add('active');
        } else {
            // Показываем вид "БЫЛО/СТАЛО"
            viewGoal.classList.remove('active');
            viewStats.classList.add('active');
            const state = btn.id === 'btn-was' ? 'was' : 'now';
            renderStats(state);
        }
    });
});



// Инициализация
renderStats('now');
