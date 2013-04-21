#ifndef PLATFORMEXTEND_H
#define PLATFORMEXTEND_H

class QWidget;

class PlatformExtend
{
public:
    PlatformExtend();
    static void notify(QWidget *w, int times = 2);
    static bool setIMEState(QWidget *widget, bool st);
};

#endif // PLATFORMEXTEND_H
