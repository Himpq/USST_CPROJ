// 家庭小管家 - 前端交互逻辑
// 通过 Fetch API 与 C++ HTTP 后端通信，实现记账功能

(function() {
  let records = [];
  let currentFilter = 'all';

  function today() {
    const d = new Date();
    return d.getFullYear() + '-' + String(d.getMonth()+1).padStart(2,'0') + '-' + String(d.getDate()).padStart(2,'0');
  }

  async function fetchRecords() {
    const date = document.getElementById('viewDate')?.value || today();
    try {
      const res = await fetch('/api/get_by_date?date=' + date);
      const text = await res.text();

      records = [];
      text.split('\n').forEach((line, i) => {
        line = line.trim();
        if (!line || i === 0) return;
        const parts = line.split(',');
        if (parts.length >= 4) {
          const spend = parseFloat(parts[2]) || 0;
          records.push({
            user: decodeURIComponent(parts[0]),
            category: decodeURIComponent(parts[1]),
            spend: spend,
            note: decodeURIComponent(parts.slice(3).join(',')),
            date: date,
            type: spend >= 0 ? 'income' : 'expense'
          });
        }
      });
    } catch (e) { records = []; }
    render();
  }

  window.addRecord = async function() {
    const type = document.getElementById('addType').value;
    const user = document.getElementById('addUser').value;
    const category = document.getElementById('addCategory').value;
    const raw = parseFloat(document.getElementById('addAmount').value);
    const note = document.getElementById('addNote').value || category;

    if (!raw || raw <= 0) return alert('金额必须大于0');
    if (!user) return alert('请填写用户');
    if (!category) return alert('请填写分类');

    const amount = type === 'expense' ? -raw : raw;

    await fetch('/api/write?user=' + encodeURIComponent(user) +
                 '&category=' + encodeURIComponent(category) +
                 '&spend=' + amount +
                 '&note=' + encodeURIComponent(note));

    document.getElementById('addUser').value = '';
    document.getElementById('addCategory').value = '';
    document.getElementById('addAmount').value = '';
    document.getElementById('addNote').value = '';
    fetchRecords();
  };

  window.setFilter = function(f) {
    currentFilter = f;
    document.querySelectorAll('.filter-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.filter === f);
    });
    render();
  };

  function render() {
    const filtered = records.filter(r => {
      if (currentFilter === 'all') return true;
      return r.type === currentFilter;
    });
    const tbody = document.getElementById('recordsBody');

    if (filtered.length === 0) {
      tbody.innerHTML = '<tr><td colspan="6" class="empty-msg">暂无记录</td></tr>';
    } else {
      tbody.innerHTML = filtered.map(r => {
        const isIncome = r.type === 'income';
        return `<tr>
          <td>${r.date}</td>
          <td><span class="type-badge ${isIncome ? 'income' : 'expense'}">${isIncome ? '收入' : '支出'}</span></td>
          <td class="${isIncome ? 'amount-income' : 'amount-expense'}">${isIncome ? '+' : ''}${r.spend.toFixed(2)}</td>
          <td>${r.user}</td>
          <td>${r.category}</td>
          <td>${r.note}</td>
        </tr>`;
      }).join('');
    }

    updateSummary();
  }

  function updateSummary() {
    let income = 0, expense = 0;
    records.forEach(r => {
      if (r.spend > 0) income += r.spend;
      else expense += -r.spend;
    });
    document.getElementById('totalIncome').textContent = income.toFixed(2);
    document.getElementById('totalExpense').textContent = expense.toFixed(2);
    document.getElementById('balance').textContent = (income - expense).toFixed(2);
  }

  fetchRecords();
  setInterval(fetchRecords, 5000);
})();
