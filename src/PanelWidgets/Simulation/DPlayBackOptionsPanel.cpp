#include "DPlayBackOptionsPanel.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>

// --- --- --- --- --- UI Items --- --- --- --- ---

const QSize DPlayBackOptionsPanel::m_gradientSize{100, 50};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DPlayBackOptionsPanel::DPlayBackOptionsPanel(QList<QLinearGradient>& gradients) : QWidget(nullptr) {
	this->ui.setupUi(this);
	this->setGradients(gradients);
}

DPlayBackOptionsPanel::~DPlayBackOptionsPanel() {
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DPlayBackOptionsPanel::setGradients(QList<QLinearGradient>& gradients) {
	QPixmap pixmap{ this->m_gradientSize };
	QPainter painter{ &pixmap };
	this->ui.style_comboBox->setIconSize(this->m_gradientSize);
	this->ui.style_comboBox->clear();

	for (QLinearGradient& gradient : gradients) {
		gradient.setStart(0, 0);
		gradient.setFinalStop(this->m_gradientSize.width(), 0);
		painter.fillRect(pixmap.rect(), { gradient });
		this->ui.style_comboBox->addItem(pixmap, "");
	}
}

void DPlayBackOptionsPanel::setEditable(bool editable) {
	this->ui.cellLabel_comboBox->setEnabled(editable);
	this->ui.style_comboBox->setEnabled(editable);
}

// --- --- --- --- --- Getter --- --- --- --- ---

QPushButton* DPlayBackOptionsPanel::getCloseButton() {
	return this->ui.close_pushButton;
}

QComboBox* DPlayBackOptionsPanel::getStyleComboBox() {
	return this->ui.style_comboBox;
}

QComboBox* DPlayBackOptionsPanel::getCellLabelComboBox() {
	return this->ui.cellLabel_comboBox;
}
