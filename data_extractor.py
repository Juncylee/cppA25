import pandas as pd
import json

# 1. 读取 CSV，跳过前17行
csv_file = 'data.csv'
df = pd.read_csv(csv_file, skiprows=17, header=0)

# 2. 保留列：B '交易类型', E '收/支', F '金额'
df = df.iloc[:, [1, 4, 5]].copy()
df.columns = ['type', 'io', 'amt']

# 3. 过滤内部流动 '/'
df = df[df['io'] != '/']

# 4. 清洗金额，去除 "¥" 和引号，并转换为数值
def clean_amt(x):
    s = str(x).replace('¥', '').replace('"', '')
    try:
        return float(s)
    except ValueError:
        return 0.0
df['amt'] = df['amt'].apply(clean_amt)

# 5. 按类型和收支汇总
income_by_type = df[df['io'].str.contains('收入')].groupby('type')['amt'].sum().to_dict()
expense_by_type = df[df['io'].str.contains('支出')].groupby('type')['amt'].sum().to_dict()
total_income = sum(income_by_type.values())
total_expense = sum(expense_by_type.values())

# 6. 生成 data.json
analysis = {
    'totalIncome': total_income,
    'totalExpense': total_expense,
    'incomeByType': income_by_type,
    'expenseByType': expense_by_type
}
with open('data.json', 'w', encoding='utf-8') as f:
    json.dump(analysis, f, ensure_ascii=False, indent=2)

# 7. **生成 data_summary.txt**
with open('data_summary.txt', 'w', encoding='utf-8') as f:
    f.write(f"总收入: ¥ {total_income}\n")
    f.write(f"总支出: ¥ {total_expense}\n\n")
    f.write('收入明细：\n')
    for k, v in income_by_type.items():
        f.write(f"  - {k}：¥ {v}\n")
    f.write('\n支出明细：\n')
    for k, v in expense_by_type.items():
        f.write(f"  - {k}：¥ {v}\n")

print('已生成 data.json 和 data_summary.txt')