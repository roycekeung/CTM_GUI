#pragma once

#include <QWidget>
#include "ui_DAddRemoveButtons.h"

class DAddRemoveButtons : public QWidget{
	Q_OBJECT

public:

	QPushButton* AddButton;
	QPushButton* RemoveButton;

	DAddRemoveButtons(QWidget *parent = Q_NULLPTR);
	~DAddRemoveButtons();

private:
	Ui::DAddRemoveButtons ui;
};
