(function() {
  let records = [];
  let currentFilter = 'all';
  let currentDateRange = 'day';
  let customStart = '';
  let customEnd = '';

  function today() {
    const d = new Date();
    return d.getFullYear() + '-' + String(d.getMonth()+1).padStart(2,'0') + '-' + String(d.getDate()).padStart(2,'0');
  }

  function formatDate(d) {
    return d.getFullYear() + '-' + String(d.getMonth()+1).padStart(2,'0') + '-' + String(d.getDate()).padStart(2,'0');
  }

  function getDatesInRange() {
    const end = new Date();
    let start = new Date();
    if (currentDateRange === 'day') {
      start = end;
    } else if (currentDateRange === 'week') {
      start.setDate(end.getDate() - 6);
    } else if (currentDateRange === 'month') {
      start.setDate(end.getDate() - 29);
    } else if (currentDateRange === 'custom') {
      if (!customStart || !customEnd) return [];
      start = new Date(customStart);
      const e = new Date(customEnd);
      const dates = [];
      for (let d = new Date(start); d <= e; d.setDate(d.getDate() + 1))
        dates.push(formatDate(d));
      return dates;
    }
    const dates = [];
    for (let d = new Date(start); d <= end; d.setDate(d.getDate() + 1))
      dates.push(formatDate(d));
    return dates;
  }

  async function fetchRecords() {
    const dates = getDatesInRange();
    if (dates.length === 0) { records = []; render(); return; }

    records = [];
    for (const date of dates) {
      try {
        const res = await fetch('/api/get_by_date?date=' + date);
        const text = await res.text();
        if (text === 'No data for this date' || text === 'Parameter is empty') continue;
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
      } catch (e) {}
    }
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
      if (btn.dataset.filter) btn.classList.toggle('active', btn.dataset.filter === f);
    });
    render();
  };

  window.setDateRange = function(r) {
    currentDateRange = r;
    document.querySelectorAll('#dateRangeGroup .filter-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.range === r);
    });
    document.getElementById('customDateRange').style.display = r === 'custom' ? 'flex' : 'none';
    if (r !== 'custom') fetchRecords();
  };

  window.applyCustomRange = function() {
    customStart = document.getElementById('startDate').value;
    customEnd = document.getElementById('endDate').value;
    if (!customStart || !customEnd) return alert('请选择起止日期');
    fetchRecords();
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
