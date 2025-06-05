from openpyxl import Workbook
 
# 创建一个Workbook对象，这相当于创建了一个Excel文件
wb = Workbook()
 
# 激活当前工作表
ws = wb.active
 
# 修改工作表的名称
ws.title = "MySheet"
 
# 写入数据到单元格
ws['A1'] = 'Hello'
ws.append([1, 2, 3])  # 在下一行追加数据
 
# 保存Excel文件
wb.save("example.xlsx")