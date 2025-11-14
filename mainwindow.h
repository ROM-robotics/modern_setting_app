#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSwipeGesture>
#include <QGestureEvent>
#include <QFrame>
#include <QScrollArea>
#include <QPainter>
#include <QResizeEvent>
#include <QDialog>
#include <QListWidget>
#include <QJsonArray>
#include <QTimer>

// Forward declaration
class Ethernet;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Custom widget for stat cards
class StatCard : public QFrame
{
    Q_OBJECT
public:
    explicit StatCard(const QString &title, const QString &value, 
                     const QString &change, const QString &iconText,
                     QWidget *parent = nullptr);
    void updateOrientation(bool isLandscape);
    void updateWifiStatus(bool connected, const QString &ssid, 
                         const QString &ip, const QString &robotName);
    void showConnecting();
    void showConnectionFailed();

private:
    QLabel *statusLabel;
    QLabel *ssidLabel;
    QLabel *ipLabel;
    QLabel *robotLabel;
    QTimer *blinkTimer;
    bool blinkState;
};

// Custom widget for activity items
class ActivityItem : public QFrame
{
    Q_OBJECT
public:
    explicit ActivityItem(const QString &title, const QString &date,
                         const QString &category, const QString &price,
                         const QColor &categoryColor, QWidget *parent = nullptr);
};

// Custom widget for donut chart
class DonutChart : public QWidget
{
    Q_OBJECT
public:
    explicit DonutChart(QWidget *parent = nullptr);
    void setData(const QVector<QPair<QString, double>> &data,
                const QVector<QColor> &colors);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<QPair<QString, double>> m_data;
    QVector<QColor> m_colors;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool event(QEvent *event) override;
    bool gestureEvent(QGestureEvent *event);
    void swipeTriggered(QSwipeGesture *gesture);
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void nextPage();
    void prevPage();
    void goToPage(int index);
    void onWifiButtonClicked();
    void onWifiListReceived(const QJsonArray &wifiList);
    void onWifiStatusReceived(const QJsonObject &status);
    void onWifiConnecting(const QString &ssid);
    void onWifiConnectionResult(bool success, const QString &message);
    void onEthernetError(const QString &errorMessage);

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QWidget *page1, *page2, *page3, *page4;
    QPushButton *prevButton, *nextButton;
    QLabel *pageIndicator;
    bool isLandscape;
    
    // Stat cards for page 1
    QGridLayout *statsLayout;
    QVector<StatCard*> statCards;
    StatCard *wifiStatusCard;
    
    // System Resources Monitor
    QFrame *resourceMonitorCard;
    QWidget *resourceGraphWidget;
    QPushButton *toggleResourceBtn;
    bool resourceMonitorVisible;
    
    // Ethernet connection
    Ethernet *m_ethernet;
    
    void setupPages();
    void setupNavigation();
    void updateNavigation();
    void animatePageTransition(int index);
    void updateLayoutOrientation();
    void toggleResourceMonitor();
    
    // Page creation helpers
    QWidget* createDashboardPage();
    QWidget* createAnalyticsPage();
    QWidget* createSubscriptionsPage();
    QWidget* createSettingsPage();
    
    // UI helper methods
    QFrame* createCard(QWidget *parent = nullptr);
    QString getCardStyle();
    QString getButtonStyle();
};
#endif // MAINWINDOW_H
