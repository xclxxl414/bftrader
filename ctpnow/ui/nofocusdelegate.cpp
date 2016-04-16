#include "nofocusdelegate.h"

void bfAdjustTableWidget(QTableWidget* tableWidget){
    // tableWidget->resizeColumnsToContents(); //自动调整列宽度=
    //tableWidget->horizontalHeader()->setStretchLastSection(true); //最后一览自适应宽度=
    //tableWidget->horizontalHeader()->setDefaultSectionSize(150); //确实列宽=
    tableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击=
    tableWidget->horizontalHeader()->setSectionsMovable(false); //设置表头不可点击=
    tableWidget->horizontalHeader()->setHighlightSections(false); //当表格只有一行的时候，则表头会出现塌陷问题=
    //tableWidget->setFrameShape(QFrame::NoFrame); //设置无边框=
    //tableWidget->setShowGrid(false); //设置不显示格子线=
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); //自适应行距=
    tableWidget->verticalHeader()->setVisible(false); //设置垂直头不可见=
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑=
    tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection); //可多选多行=
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选择行为时每次选择一行=
    tableWidget->setItemDelegate(new NoFocusDelegate()); // 去鼠标点击出现的虚框=
    tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background-color:#FFFFFF;}"); //设置表头背景色=
}
