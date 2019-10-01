/*-
 * Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "qcounter.h"

QcSolidColor :: QcSolidColor() : QPushButton(QString(""))
{
	connect(this, SIGNAL(released()), this, SLOT(handle_released()));
	setColor(QColor(0,0,0));
}

void
QcSolidColor :: setColor(const QColor newcolor)
{
	color = newcolor;
	update();
}

void
QcSolidColor :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);

	paint.fillRect(event->rect(), QBrush(color));
}

void
QcSolidColor :: handle_released()
{
	QColorDialog *diag = new QColorDialog(color, this);

	if (diag->exec() == QDialog::Accepted) {
		setColor(diag->currentColor());
		changed();
	}
	delete diag;
}

QcConfigEntries :: QcConfigEntries() :
	QGroupBox(), QGridLayout(this)
{
	int x;

	setTitle(QGroupBox::tr("Available series"));

	addWidget(new QLabel(QGroupBox::tr("Start")), 0, 0, 1, 1);
	addWidget(new QLabel(QGroupBox::tr("Stop")), 0, 1, 1, 1);
	addWidget(new QLabel(QGroupBox::tr("Description")), 0, 2, 1, 1);
	addWidget(new QLabel(QGroupBox::tr("Color")), 0, 3, 1, 1);

	for (x = 0; x != QB_MAX_SERIES; x++) {
		qb_color[x] = new QcSolidColor();
		led_text[x] = new QLineEdit();
		spn_start[x] = new QSpinBox();
		spn_stop[x] = new QSpinBox();
		spn_start[x]->setRange(-1,9999);
		spn_start[x]->setValue(-1);
		spn_stop[x]->setRange(-1,9999);
		spn_stop[x]->setValue(-1);
		addWidget(spn_start[x], x+1, 0, 1, 1);
		addWidget(spn_stop[x], x+1, 1, 1, 1);
		addWidget(led_text[x], x+1, 2, 1, 1);
		addWidget(qb_color[x], x+1, 3, 1, 1);
	}
}

QcConfigTab :: QcConfigTab(QcMainWindow *_mw)
{
	int x;

	mw = _mw;

	num_free = 0;
	num_used = 0;

	TAILQ_INIT(&head_free);
	TAILQ_INIT(&head_used);

	curr_font.setPointSize(48);

	main_gl = new QGridLayout(this);
	cfg_entries = new QcConfigEntries();
	for (x = 0; x != QB_MAX_SERIES; x++) {
		connect(cfg_entries->qb_color[x], SIGNAL(changed()), this, SLOT(handle_changed()));
		connect(cfg_entries->led_text[x], SIGNAL(textChanged(const QString &)), this, SLOT(handle_changed()));
	}
	cfg_generate = new QPushButton(tr("Generate"));
	connect(cfg_generate, SIGNAL(released()), this, SLOT(handle_generate()));

	lbl_status = new QLabel();

	cfg_font = new QPushButton(tr("Font"));
	connect(cfg_font, SIGNAL(released()), this, SLOT(handle_font()));

	cfg_series = new QSpinBox();
	cfg_series->setRange(0,QB_MAX_HISTORY);
	connect(cfg_series, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));
	connect(cfg_series, SIGNAL(valueChanged(int)), this, SLOT(handle_series(int)));

	cfg_history = new QSpinBox();
	cfg_history->setRange(0,QB_MAX_HISTORY);
	cfg_history->setValue(4);
	connect(cfg_history, SIGNAL(valueChanged(int)), this, SLOT(handle_changed()));

	cfg_repetitions = new QCheckBox();

	cfg_draw_speed = new QSpinBox();
	cfg_draw_speed->setRange(25,1000);
	cfg_draw_speed->setValue(250);
	cfg_draw_speed->setSuffix(" ms");

	main_gl->addWidget(cfg_entries, 0,0,1,4);

	main_gl->addWidget(new QLabel(tr("History length")),1,0,1,2);
	main_gl->addWidget(cfg_history,1,2,1,1);

	main_gl->addWidget(new QLabel(tr("Series length")),2,0,1,2);
	main_gl->addWidget(cfg_series,2,2,1,1);

	main_gl->addWidget(new QLabel(tr("Allow repeptions when series length > 0")),3,0,1,2);
	main_gl->addWidget(cfg_repetitions,3,2,1,1);

	main_gl->addWidget(new QLabel(tr("Card drawing speed")),4,0,1,2);
	main_gl->addWidget(cfg_draw_speed,4,2,1,1);

	main_gl->addWidget(cfg_generate, 5, 0, 1, 1);
	main_gl->addWidget(lbl_status, 5, 1, 1, 1);
	main_gl->addWidget(cfg_font, 5, 2, 1, 1);

	main_gl->setRowStretch(4, 1);
	main_gl->setColumnStretch(1, 1);
}

QcCard *
QcConfigTab :: newCard()
{
	return new QcCard();
}

void
QcConfigTab :: freeCard(QcCard *pc)
{
	delete pc;
}

QcCard *
QcConfigTab :: drawCard(int final)
{
	QcCard *pc;
	int value;

	if (num_free == 0)
		return (0);

#ifdef WIN32
	value = qrand() % (uint32_t)num_free;
#else
	value = arc4random() % (uint32_t)num_free;
#endif
	if (value < 0)
		value = (num_free + value) % value;

	TAILQ_FOREACH(pc, &head_free, entry) {
		if (!value--)
			break;
	}

	if (pc != 0 && final != 0) {
		TAILQ_REMOVE(&head_free, pc, entry);
		TAILQ_INSERT_HEAD(&head_used, pc, entry);
		num_free--;
		num_used++;
	}
	return (pc);
}

QcCard *
QcConfigTab :: undoCard(int final)
{
	QcCard *pc;

	pc = TAILQ_FIRST(&head_used);

	if (pc != 0 && final != 0) {
		TAILQ_REMOVE(&head_used, pc, entry);
		TAILQ_INSERT_TAIL(&head_free, pc, entry);
		num_free++;
		num_used--;
	}
	return (pc);
}

void
QcConfigTab :: handle_generate()
{
	QcCard *pc;
	int x;
	int y;

	num_free = 0;
	num_used = 0;
	mw->digits = 0;
	mw->fullscreen->blanked = 1;

	while ((pc = TAILQ_FIRST(&head_free)) != 0) {
		TAILQ_REMOVE(&head_free, pc, entry);
		freeCard(pc);
	}
	while ((pc = TAILQ_FIRST(&head_used)) != 0) {
		TAILQ_REMOVE(&head_used, pc, entry);
		freeCard(pc);
	}

	for (x = 0; x != QB_MAX_SERIES; x++) {
		int start = cfg_entries->spn_start[x]->value();
		int stop = cfg_entries->spn_stop[x]->value() + 1;
		if (start >= stop)
			continue;
		if (stop > 1000) {
			if (mw->digits < 4)
				mw->digits = 4;
		} else if (stop > 100) {
			if (mw->digits < 3)
				mw->digits = 3;
		} else if (stop > 10) {
			if (mw->digits < 2)
				mw->digits = 2;
		} else {
			if (mw->digits < 1)
				mw->digits = 1;
		}
		for (y = start; y != stop; y++) {
			if (y < 0)
				continue;
			pc = newCard();
			pc->value = y;
			pc->which = x;
			TAILQ_INSERT_TAIL(&head_free, pc, entry);
			num_free++;
		}
	}

	lbl_status->setText(tr("Generated %1 cards").arg(num_free));
	handle_changed();
}

void
QcConfigTab :: handle_font()
{
	QFontDialog *diag = new QFontDialog(this);

	diag->setCurrentFont(curr_font);

	if (diag->exec() == QDialog::Accepted) {
		curr_font = diag->selectedFont();
		handle_changed();
	}
}

void
QcConfigTab :: handle_changed()
{
	mw->control_tab->handle_redraw();
}

void
QcConfigTab :: handle_series(int value)
{
	if (value > 0)
		cfg_history->setValue(value - 1);
}

QcControlTab :: QcControlTab(QcMainWindow *_mw)
{
	mw = _mw;

	main_gl = new QGridLayout(this);

	val_main = new QcShowValue(_mw);

	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(handle_timer()));

	timer_count = 0;
	last_history = 0;

	controls_gb = new QcGroupBox(tr("Controls"));

	but_draw = new QPushButton(tr("Draw"));
	connect(but_draw, SIGNAL(released()), this, SLOT(handle_draw()));

	but_undo = new QPushButton(tr("Undo"));
	connect(but_undo, SIGNAL(released()), this, SLOT(handle_undo()));

	but_show = new QPushButton(tr("Show"));
	connect(but_show, SIGNAL(released()), this, SLOT(handle_show()));

	but_generate = new QPushButton(tr("Generate"));
	but_toggle_blank = new QPushButton(tr("Toggle blank"));

	controls_gb->addWidget(but_draw, 0, 0, 1, 1);
	controls_gb->addWidget(but_undo, 0, 3, 1, 1);
	controls_gb->addWidget(but_show, 0, 4, 1, 1);
	controls_gb->addWidget(but_generate, 0, 5, 1, 1);
	controls_gb->addWidget(but_toggle_blank, 0, 6, 1, 1);
	controls_gb->setColumnStretch(1,1);

	main_gl->addWidget(val_main, 0,0,1,1);
	main_gl->addWidget(controls_gb, 1,0,1,1);
	main_gl->setRowStretch(0,1);
	main_gl->setColumnStretch(0,1);
}

void
QcControlTab :: handle_draw()
{
	if (mw->config_tab->cfg_repetitions->isChecked()) {
		int used = mw->config_tab->num_used;
		int series = mw->config_tab->cfg_series->value();

		if (series > 0 && (used % series) == 0) {
			while (used--)
				mw->config_tab->undoCard(1);
		}
	}
	if (mw->config_tab->num_free == 0) {
		QString title("Quick Counter");
		QString msg("All cards are drawn. Try to generate new cards.");
		QMessageBox box(QMessageBox::Warning, title, msg);
		box.setWindowIcon(QIcon(QString(":/qcounter_48x48.png")));
		box.exec();
	} else {
		mw->fullscreen->blanked = 0;
		timer_count = 0;
		timer->start(mw->config_tab->cfg_draw_speed->value());
	}
}

void
QcControlTab :: handle_undo()
{
	timer_count = 0;
	timer->stop();

	mw->config_tab->undoCard(1);
	mw->config_tab->handle_changed();
}

void
QcControlTab :: handle_show()
{
	mw->fullscreen->showNormal();
}

void
QcControlTab :: handle_redraw()
{
	QcCard *pc = TAILQ_FIRST(&mw->config_tab->head_used);
	int series = mw->config_tab->cfg_series->value();
	int history = mw->config_tab->cfg_history->value();
	int used = mw->config_tab->num_used;
	int x;

	if (history != last_history) {
		last_history = history;

		for (x = 0; x != QB_MAX_HISTORY; x++) {
			mw->fullscreen->history_gl->removeWidget(mw->fullscreen->val_history[x]);
			mw->fullscreen->history_gl->setRowStretch(x,0);
		}

		for (x = 0; x != history; x++) {
			mw->fullscreen->history_gl->addWidget(mw->fullscreen->val_history[x],x,0,1,1);
			mw->fullscreen->history_gl->setRowStretch(x,1);
		}
	}

	if (pc != 0) {
		mw->control_tab->val_main->card = *pc;
		mw->control_tab->val_main->show();
		mw->control_tab->val_main->update();
	} else {
		mw->control_tab->val_main->hide();
	}

	if (mw->fullscreen->blanked == 0 && pc != 0) {
		mw->fullscreen->val_main->card = *pc;
		mw->fullscreen->val_main->show();

		uint8_t map[QB_MAX_HISTORY];

		memset(map, 0, sizeof(map));

		for (x = 0; x != QB_MAX_HISTORY; x++) {
			int off;

			if (series > 0)
				off = (used - x - 1) % series;
			else
				off = x;

			pc = TAILQ_NEXT(pc, entry);

			/* check if we enter previous series */
			if (pc == 0 || (series > 0 && off == 0))
				break;

			if (series > 0)
				off--;

			if (off < history) {
				mw->fullscreen->val_history[off]->card = *pc;
				mw->fullscreen->val_history[off]->show();
				map[off] = 1;
			}
		}
		for (x = 0; x != QB_MAX_HISTORY; x++) {
			if (map[x] == 0)
				mw->fullscreen->val_history[x]->hide();
		}
	} else {
		mw->fullscreen->val_main->hide();

		for (x = 0; x != QB_MAX_HISTORY; x++)
			mw->fullscreen->val_history[x]->hide();
	}
	mw->fullscreen->update();
}

void
QcControlTab :: handle_timer()
{
	QcCard *pc = mw->config_tab->drawCard(1);

	handle_redraw();

	if (timer_count != (3 * 4)) {
		if (pc != 0)
			mw->config_tab->undoCard(1);
	} else {
		timer->stop();
	}
	timer_count++;
}

QcShowValue :: QcShowValue(QcMainWindow *_mw)
{
	mw = _mw;
	card.value = 0;
	card.which = 0;
}

void
QcShowValue :: paintEvent(QPaintEvent *event)
{
	QPainter paint(this);
	QString desc;
	int x;

	switch (mw->digits) {
	case 1:
		desc = QString("%1").arg(card.value % 10);
		break;
	case 2:
		desc = QString("%1%2").arg((card.value / 10) % 10)
		  .arg(card.value % 10);
		break;
	case 3:
		desc = QString("%1%2%3").arg((card.value / 100) % 10)
		  .arg((card.value / 10) % 10).arg(card.value % 10);
		break;
	case 4:
		desc = QString("%1%2%3%4").arg((card.value / 1000) % 10)
		  .arg((card.value / 100) % 10).arg((card.value / 10) % 10)
		  .arg(card.value % 10);
		break;
	default:
		desc = QString("?");
		break;
	}

	QString str = mw->config_tab->cfg_entries->led_text[card.which]->text();
	if (!str.isEmpty()) {
		desc += "\n";
		desc += str;
	}

	QColor cr(mw->config_tab->cfg_entries->qb_color[card.which]->color);

	QColor ca,cb;

	if (cr == QColor(255,255,255)) {
		ca = QColor(0,0,0);
		cb = cr;
	} else if (cr == QColor(0,0,0)) {
		ca = QColor(255,255,255);
		cb = cr;
	} else {
		ca = cr.lighter();
		cb = cr.darker();
	}

	paint.setBrush(cb);
	paint.setPen(cb);
	paint.drawRoundedRect(contentsRect(), 20, 20);

	paint.setBrush(ca);
	paint.setPen(ca);

	QTextOption opt;
	opt.setAlignment(Qt::AlignRight|Qt::AlignTop);

	QFont font = mw->config_tab->curr_font;
	QRectF limit = contentsRect();

	for (x = 0; x != 14; x++) {
		font.setPixelSize(1 << x);
		paint.setFont(font);
		QRectF r = paint.boundingRect(limit, desc, opt);
		if (r.width() > limit.width() || r.height() > limit.height())
			break;
	}
	if (x != 0)
		x--;

	opt.setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	font.setPixelSize(1 << x);
	paint.setFont(font);
	paint.drawText(contentsRect(), desc,  opt);
}

QcFullScreen :: QcFullScreen(QcMainWindow *_mw)
{
	int x;

	setWindowTitle(tr("Quick Random Counter"));
	setWindowIcon(QIcon(QString(":/qcounter_48x48.png")));

	mw = _mw;
	blanked = 1;

	main_gl = new QGridLayout(this);

	history_gl = new QcGridLayout();

	val_main = new QcShowValue(_mw);
	val_main->hide();

	for (x = 0; x != QB_MAX_HISTORY; x++) {
		val_history[x] = new QcShowValue(_mw);
		val_history[x]->hide();
	}

	main_gl->addWidget(history_gl, 0, 0, 1, 1);
	main_gl->addWidget(val_main, 0, 1, 1, 1);

	main_gl->setColumnStretch(0, 1);
	main_gl->setColumnStretch(1, 2);
}

void
QcFullScreen :: paintEvent(QPaintEvent *event)
{
	if (blanked != 0) {
		QPainter paint(this);
		paint.fillRect(event->rect(), QBrush(QColor(0,0,0)));
	} else {
		QWidget::paintEvent(event);
	}
}

void
QcFullScreen :: keyPressEvent(QKeyEvent *key)
{
	if (key->key() == Qt::Key_Escape) {
		Qt::WindowStates state = windowState();
		if (state & Qt::WindowFullScreen) {
			setWindowState(state ^ Qt::WindowFullScreen);
			key->accept();
		}
	}
}

void
QcFullScreen :: mouseDoubleClickEvent(QMouseEvent *event)
{
	/* toggle fullscreen */
	setWindowState(windowState() ^ Qt::WindowFullScreen);

	event->accept();
}

void
QcFullScreen :: handle_toggle_blank()
{
	blanked = blanked ? 0 : 1;
	mw->control_tab->handle_redraw();
}

QcMainWindow :: QcMainWindow()
{
	setWindowTitle(tr("Quick Random Counter"));
	setWindowIcon(QIcon(QString(":/qcounter_48x48.png")));

	digits = 0;

	config_tab = new QcConfigTab(this);
	control_tab = new QcControlTab(this);
	fullscreen = new QcFullScreen(this);

	connect(control_tab->but_generate, SIGNAL(released()), config_tab, SLOT(handle_generate()));
	connect(control_tab->but_toggle_blank, SIGNAL(released()), fullscreen, SLOT(handle_toggle_blank()));

	insertTab(0, control_tab, tr("Controls"));
	insertTab(1, config_tab, tr("Configuration"));
}

Q_DECL_EXPORT int
main(int argc, char **argv)
{
	QApplication app(argc, argv);

#ifdef WIN32
	/* XXX hack */
	QTime t = QTime::currentTime();
	qsrand(t.minute() * 60000 + t.second() * 1000 + t.msec());
#else
	arc4random_stir();
#endif

	/* set consistent double click interval */
	app.setDoubleClickInterval(250);

	QcMainWindow *mw = new QcMainWindow();

	mw->show();

	return (app.exec());
}
