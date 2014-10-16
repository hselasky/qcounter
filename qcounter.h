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

#ifndef _QCOUNTER_H_
#define	_QCOUNTER_H_

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <sys/queue.h>

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>
#include <QFontDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QLineEdit>
#include <QTimer>
#include <QSpinBox>
#include <QColorDialog>
#include <QTabWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QProcess>
#include <QThread>
#include <QTime>
#include <QCheckBox>

#define	QB_MAX_SERIES 8
#define	QB_MAX_HISTORY 16

class QcMainWindow;

struct QcCard {
	TAILQ_ENTRY(QcCard) entry;
	int which;
	int value;
};

typedef TAILQ_HEAD(,QcCard) QcCardHeadT;

class QcGridLayout : public QWidget, public QGridLayout
{
public:
	QcGridLayout() : QWidget(), QGridLayout(this) { };
	~QcGridLayout() { };
};

class QcGroupBox : public QGroupBox, public QGridLayout
{
public:
	QcGroupBox(const QString &title) : QGroupBox(title),
	    QGridLayout(this) { };
	~QcGroupBox() { };
};

class QcSleep : public QThread
{
public:
	static void usleep(unsigned long usecs) { QThread::usleep(usecs); }
	static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
	static void sleep(unsigned long secs) { QThread::sleep(secs); }
};

class QcSolidColor : public QPushButton
{
	Q_OBJECT;
public:
	QcSolidColor();
	~QcSolidColor() { };

	void setColor(const QColor);

	void paintEvent(QPaintEvent *);

	QColor color;

public slots:
	void handle_released();
signals:
	void changed();
};

class QcConfigEntries : public QGroupBox, public QGridLayout
{
public:
	QcConfigEntries();
	~QcConfigEntries() { };
	QcSolidColor *qb_color[QB_MAX_SERIES];
	QLineEdit *led_text[QB_MAX_SERIES];
	QSpinBox *spn_start[QB_MAX_SERIES];
	QSpinBox *spn_stop[QB_MAX_SERIES];
};

class QcShowValue : public QWidget
{
public:
	QcShowValue(QcMainWindow *);
	~QcShowValue() { };

	QcMainWindow *mw;

	QcCard card;

	void paintEvent (QPaintEvent *);
};

class QcConfigTab : public QWidget
{
	Q_OBJECT;
public:
	QcConfigTab(QcMainWindow *);
	~QcConfigTab() { };

	QcCard *drawCard(int);
	QcCard *undoCard(int);
	QcCard *newCard();
	void freeCard(QcCard *);

	QFont curr_font;

	QProcess play;

	QcCardHeadT head_free;
	QcCardHeadT head_used;

	int num_free;
	int num_used;

	QcMainWindow *mw;

	QGridLayout *main_gl;
	QcConfigEntries *cfg_entries;
	QPushButton *cfg_generate;
	QPushButton *cfg_font;
	QSpinBox *cfg_series;
	QSpinBox *cfg_history;
	QCheckBox *cfg_repetitions;
	QLabel *lbl_status;

public slots:
	void handle_generate();
	void handle_font();
	void handle_changed();
	void handle_series(int);
};

class QcControlTab : public QWidget
{
	Q_OBJECT;
public:
	QcControlTab(QcMainWindow *);
	~QcControlTab() { };

	QcMainWindow *mw;

	QGridLayout *main_gl;

	QcShowValue *val_main;

	QcGroupBox *snapshot_gb;
	QcGroupBox *controls_gb;

	QPushButton *but_draw;
	QPushButton *but_undo;
	QPushButton *but_show;

	QTimer *timer;

	int timer_count;
	int last_history;

public slots:
	void handle_redraw();
	void handle_draw();
	void handle_undo();
	void handle_show();
	void handle_timer();
};

class QcFullScreen : public QWidget
{
public:
	QcFullScreen(QcMainWindow *);
	~QcFullScreen() { };
	void keyPressEvent(QKeyEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);

	QcMainWindow *mw;

	QGridLayout *main_gl;
	QcGridLayout *history_gl;

	QcShowValue *val_main;
	QcShowValue *val_history[QB_MAX_HISTORY];
};

class QcMainWindow : public QTabWidget
{
public:
	QcMainWindow();
	~QcMainWindow() { };

	QcConfigTab *config_tab;
	QcControlTab *control_tab;

	QcFullScreen *fullscreen;

	int digits;
};

#endif		/* _QCOUNTER_H_ */
