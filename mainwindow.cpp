#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "communication/ethernet.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QToolButton>

// ========== StatCard Implementation ==========
StatCard::StatCard(const QString &title, const QString &value, 
                   const QString &change, const QString &iconText,
                   QWidget *parent)
    : QFrame(parent)
    , statusLabel(nullptr)
    , ssidLabel(nullptr)
    , ipLabel(nullptr)
    , robotLabel(nullptr)
    , blinkTimer(nullptr)
    , blinkState(false)
{
    setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    padding: 8px 16px;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // WiFi Status and Connection Status in one line
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(6);
    
    QLabel *titleLabel = new QLabel("WiFi Status:");
    titleLabel->setStyleSheet("color: #64748b; font-size: 13px; font-weight: 600;");
    
    statusLabel = new QLabel("Disconnected");
    statusLabel->setStyleSheet("color: #ef4444; font-size: 13px; font-weight: 700;");
    
    statusLayout->addWidget(titleLabel);
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    
    layout->addLayout(statusLayout);
    
    // Show connection details (initially hidden)
    ssidLabel = new QLabel("SSID: --");
    ssidLabel->setStyleSheet("color: #475569; font-size: 11px; font-weight: 500;");
    ssidLabel->hide();
    
    ipLabel = new QLabel("IP: --");
    ipLabel->setStyleSheet("color: #475569; font-size: 11px; font-weight: 500;");
    ipLabel->hide();
    
    robotLabel = new QLabel("Robot: --");
    robotLabel->setStyleSheet("color: #475569; font-size: 11px; font-weight: 500;");
    robotLabel->hide();
    
    layout->addWidget(ssidLabel);
    layout->addWidget(ipLabel);
    layout->addWidget(robotLabel);
    
    // Setup blink timer
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]() {
        blinkState = !blinkState;
        if (blinkState) {
            statusLabel->setStyleSheet("color: #f59e0b; font-size: 13px; font-weight: 700;");
        } else {
            statusLabel->setStyleSheet("color: #fbbf24; font-size: 13px; font-weight: 700;");
        }
    });
}

void StatCard::updateOrientation(bool isLandscape)
{
    // WiFi status card compact height (reduced by 1/3)
    setFixedHeight(53);
}

void StatCard::updateWifiStatus(bool connected, const QString &ssid, 
                                const QString &ip, const QString &robotName)
{
    // Stop blink timer if running
    if (blinkTimer && blinkTimer->isActive()) {
        blinkTimer->stop();
    }
    
    if (connected) {
        statusLabel->setText("Connected");
        statusLabel->setStyleSheet("color: #10b981; font-size: 13px; font-weight: 700;");
        
        ssidLabel->setText(QString("SSID: %1").arg(ssid.isEmpty() ? "--" : ssid));
        ipLabel->setText(QString("IP: %1").arg(ip.isEmpty() ? "--" : ip));
        robotLabel->setText(QString("Robot: %1").arg(robotName.isEmpty() ? "--" : robotName));
        
        ssidLabel->show();
        ipLabel->show();
        robotLabel->show();
    } else {
        statusLabel->setText("Disconnected");
        statusLabel->setStyleSheet("color: #ef4444; font-size: 13px; font-weight: 700;");
        
        ssidLabel->setText("SSID: --");
        ipLabel->setText("IP: --");
        robotLabel->setText(QString("Robot: %1").arg(robotName.isEmpty() ? "--" : robotName));
        
        // Still show robot name even when disconnected
        ssidLabel->hide();
        ipLabel->hide();
        robotLabel->show();
    }
}

void StatCard::showConnecting()
{
    // Hide detail labels during connection
    ssidLabel->hide();
    ipLabel->hide();
    robotLabel->hide();
    
    // Initialize blink state
    blinkState = true;
    
    // Create timer if not exists
    if (!blinkTimer) {
        blinkTimer = new QTimer(this);
        connect(blinkTimer, &QTimer::timeout, this, [this]() {
            blinkState = !blinkState;
            
            // Cycle through "Connecting.", "Connecting..", "Connecting..."
            static int dotCount = 1;
            QString dots = QString(".").repeated(dotCount);
            statusLabel->setText(QString("Connecting%1").arg(dots));
            
            dotCount++;
            if (dotCount > 3) {
                dotCount = 1;
            }
            
            // Keep orange color during connecting
            statusLabel->setStyleSheet("color: #f59e0b; font-size: 13px; font-weight: 700;");
        });
    }
    
    // Start blinking (500ms interval)
    statusLabel->setText("Connecting.");
    statusLabel->setStyleSheet("color: #f59e0b; font-size: 13px; font-weight: 700;");
    blinkTimer->start(500);
}

void StatCard::showConnectionFailed()
{
    // Stop blink timer if running
    if (blinkTimer && blinkTimer->isActive()) {
        blinkTimer->stop();
    }
    
    // Show failed status
    statusLabel->setText("Failed to connect");
    statusLabel->setStyleSheet("color: #ef4444; font-size: 13px; font-weight: 700;");
    
    // Hide details
    ssidLabel->hide();
    ipLabel->hide();
    
    // Keep robot name visible
    robotLabel->show();
}

// ========== ActivityItem Implementation ==========
ActivityItem::ActivityItem(const QString &title, const QString &date,
                          const QString &category, const QString &price,
                          const QColor &categoryColor, QWidget *parent)
    : QFrame(parent)
{
    setStyleSheet(
        "QFrame {"
        "    background-color: transparent;"
        "    border-bottom: 1px solid #e2e8f0;"
        "    padding: 12px 0px;"
        "}"
    );
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    
    // Left side - Title and date
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(4);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #0f172a; font-size: 14px; font-weight: 600;");
    
    QLabel *dateLabel = new QLabel(date);
    dateLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");
    
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(dateLabel);
    
    // Category badge
    QLabel *categoryLabel = new QLabel(category);
    categoryLabel->setStyleSheet(QString(
        "background-color: %1; color: white; "
        "border-radius: 10px; padding: 4px 12px; "
        "font-size: 11px; font-weight: 500;"
    ).arg(categoryColor.name()));
    categoryLabel->setMaximumHeight(24);
    
    // Price
    QLabel *priceLabel = new QLabel(price);
    priceLabel->setStyleSheet("color: #0f172a; font-size: 15px; font-weight: 600;");
    
    layout->addLayout(leftLayout);
    layout->addWidget(categoryLabel);
    layout->addStretch();
    layout->addWidget(priceLabel);
}

// ========== DonutChart Implementation ==========
DonutChart::DonutChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(200, 200);
}

void DonutChart::setData(const QVector<QPair<QString, double>> &data,
                        const QVector<QColor> &colors)
{
    m_data = data;
    m_colors = colors;
    update();
}

void DonutChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int size = qMin(width(), height());
    int margin = 20;
    QRect rect(margin, margin, size - 2*margin, size - 2*margin);
    
    double total = 0;
    for (const auto &item : m_data) {
        total += item.second;
    }
    
    int startAngle = 90 * 16; // Start from top
    
    for (int i = 0; i < m_data.size(); ++i) {
        int spanAngle = -static_cast<int>((m_data[i].second / total) * 360 * 16);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_colors[i]);
        painter.drawPie(rect, startAngle, spanAngle);
        
        startAngle += spanAngle;
    }
    
    // Draw inner circle (donut hole)
    int innerSize = size * 0.5;
    int innerMargin = (size - innerSize) / 2;
    QRect innerRect(innerMargin, innerMargin, innerSize, innerSize);
    painter.setBrush(QColor("#f8fafc"));
    painter.drawEllipse(innerRect);
}

// ========== MainWindow Implementation ==========
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , stackedWidget(nullptr)
    , page1(nullptr), page2(nullptr), page3(nullptr), page4(nullptr)
    , prevButton(nullptr), nextButton(nullptr)
    , pageIndicator(nullptr)
    , isLandscape(true)
    , statsLayout(nullptr)
    , wifiStatusCard(nullptr)
    , resourceMonitorCard(nullptr)
    , resourceGraphWidget(nullptr)
    , toggleResourceBtn(nullptr)
    , resourceMonitorVisible(true)
    , m_ethernet(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Android Robot App");
    
    // Android tablet landscape size (standard 10" tablet)
    setFixedSize(1280, 800);
    
    // Set application-wide font to SF Pro Text (fallback to Roboto)
    QFont appFont("SF Pro Text, Roboto, -apple-system, system-ui, sans-serif");
    appFont.setPointSize(10);
    QApplication::setFont(appFont);
    
    // Initialize Ethernet connection
    m_ethernet = new Ethernet(this);
    connect(m_ethernet, &Ethernet::connected, this, [this]() {
        qDebug() << "MainWindow: Ethernet connected, requesting WiFi status";
    });
    connect(m_ethernet, &Ethernet::wifiListReceived, this, &MainWindow::onWifiListReceived);
    connect(m_ethernet, &Ethernet::wifiStatusReceived, this, &MainWindow::onWifiStatusReceived);
    connect(m_ethernet, &Ethernet::error, this, &MainWindow::onEthernetError);
    
    setupPages();
    setupNavigation();
    
    // Connect to robot after UI is set up
    m_ethernet->connectToRobot("10.0.0.5", 8888);
    
    // Enable swipe gestures
    grabGesture(Qt::SwipeGesture);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getCardStyle()
{
    return "QFrame {"
           "    background-color: white;"
           "    border-radius: 12px;"
           "    padding: 20px;"
           "}";
}

QString MainWindow::getButtonStyle()
{
    return "QPushButton {"
           "    background-color: #3b82f6;"
           "    color: white;"
           "    border: none;"
           "    padding: 10px 24px;"
           "    font-size: 14px;"
           "    font-weight: 600;"
           "    border-radius: 8px;"
           "}"
           "QPushButton:hover {"
           "    background-color: #2563eb;"
           "}"
           "QPushButton:pressed {"
           "    background-color: #1d4ed8;"
           "}"
           "QPushButton:disabled {"
           "    background-color: #cbd5e1;"
           "    color: #94a3b8;"
           "}";
}

QFrame* MainWindow::createCard(QWidget *parent)
{
    QFrame *card = new QFrame(parent);
    card->setStyleSheet(getCardStyle());
    return card;
}

QWidget* MainWindow::createDashboardPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8fafc;");
    
    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: transparent;");
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    
    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Robot Computer Statistics");
    titleLabel->setStyleSheet("color: #0f172a; font-size: 24px; font-weight: 700;");
    
    QPushButton *addBtn = new QPushButton(" Wifi");
    QPixmap wifiPixmap(":/images/wifi_128_white.png");
    qDebug() << "WiFi icon path: :/images/wifi_128_white.png";
    qDebug() << "Icon loaded:" << !wifiPixmap.isNull();
    qDebug() << "Icon size:" << wifiPixmap.size();
    if (!wifiPixmap.isNull()) {
        addBtn->setIcon(QIcon(wifiPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        addBtn->setIconSize(QSize(20, 20));
    } else {
        qDebug() << "Failed to load WiFi icon!";
    }
    addBtn->setStyleSheet(getButtonStyle());
    addBtn->setMaximumWidth(120);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onWifiButtonClicked);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(addBtn);
    
    mainLayout->addLayout(headerLayout);
    
    // Stats cards
    statsLayout = new QGridLayout();
    statsLayout->setSpacing(16);
    
    wifiStatusCard = new StatCard("Active Subscriptions", "10", "+2 new this month", "💳");
    
    statCards = {wifiStatusCard};
    
    statsLayout->addWidget(wifiStatusCard, 0, 0);
    
    mainLayout->addLayout(statsLayout);
    
    // WiFi status will be automatically requested when ethernet connects
    
    // System Resources Monitor Card
    resourceMonitorCard = createCard();
    QVBoxLayout *resourceLayout = new QVBoxLayout(resourceMonitorCard);
    resourceLayout->setSpacing(3);
    resourceLayout->setContentsMargins(16, 6, 16, 6);
    
    // Header with toggle button
    QHBoxLayout *resourceHeaderLayout = new QHBoxLayout();
    QLabel *resourceTitle = new QLabel("System Resources Monitor");
    resourceTitle->setStyleSheet("color: #0f172a; font-size: 16px; font-weight: 600;");
    
    toggleResourceBtn = new QPushButton("▼ Hide");
    toggleResourceBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9;"
        "    color: #475569;"
        "    border: none;"
        "    padding: 4px 12px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e2e8f0;"
        "}"
    );
    toggleResourceBtn->setMaximumWidth(80);
    connect(toggleResourceBtn, &QPushButton::clicked, this, &MainWindow::toggleResourceMonitor);
    
    resourceHeaderLayout->addWidget(resourceTitle);
    resourceHeaderLayout->addStretch();
    resourceHeaderLayout->addWidget(toggleResourceBtn);
    
    resourceLayout->addLayout(resourceHeaderLayout);
    
    // Graph widget container
    resourceGraphWidget = new QWidget();
    resourceGraphWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *graphLayout = new QVBoxLayout(resourceGraphWidget);
    graphLayout->setSpacing(3);
    graphLayout->setContentsMargins(0, 2, 0, 0);
    
    // CPU Section
    QLabel *cpuLabel = new QLabel("CPU");
    cpuLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: 600;");
    graphLayout->addWidget(cpuLabel);
    
    // CPU Graph placeholder
    QWidget *cpuGraphWidget = new QWidget();
    cpuGraphWidget->setFixedHeight(80);
    cpuGraphWidget->setStyleSheet(
        "background-color: #f8fafc; "
        "border: 1px solid #e2e8f0; "
        "border-radius: 6px;"
    );
    graphLayout->addWidget(cpuGraphWidget);
    
    // Memory and Swap Section
    QLabel *memoryLabel = new QLabel("Memory and Swap");
    memoryLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: 600;");
    graphLayout->addWidget(memoryLabel);
    
    // Memory Graph placeholder
    QWidget *memoryGraphWidget = new QWidget();
    memoryGraphWidget->setFixedHeight(60);
    memoryGraphWidget->setStyleSheet(
        "background-color: #f8fafc; "
        "border: 1px solid #e2e8f0; "
        "border-radius: 6px;"
    );
    graphLayout->addWidget(memoryGraphWidget);
    
    // Network Section
    QLabel *networkLabel = new QLabel("Network");
    networkLabel->setStyleSheet("color: #64748b; font-size: 11px; font-weight: 600;");
    graphLayout->addWidget(networkLabel);
    
    // Network Graph placeholder
    QWidget *networkGraphWidget = new QWidget();
    networkGraphWidget->setFixedHeight(60);
    networkGraphWidget->setStyleSheet(
        "background-color: #f8fafc; "
        "border: 1px solid #e2e8f0; "
        "border-radius: 6px;"
    );
    graphLayout->addWidget(networkGraphWidget);
    
    resourceLayout->addWidget(resourceGraphWidget);
    
    mainLayout->addWidget(resourceMonitorCard);
    mainLayout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    
    return page;
}

QWidget* MainWindow::createAnalyticsPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8fafc;");
    
    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(20);
    layout->setContentsMargins(24, 24, 24, 24);
    
    QLabel *titleLabel = new QLabel("Robot Mode");
    titleLabel->setStyleSheet("color: #0f172a; font-size: 24px; font-weight: 700;");
    layout->addWidget(titleLabel);
    
    // Analytics cards
    QGridLayout *analyticsGrid = new QGridLayout();
    analyticsGrid->setSpacing(16);
    
    StatCard *avgCard = new StatCard("Average Monthly Cost", "$110.36", "+8% trend", "📈");
    StatCard *savingsCard = new StatCard("Potential Savings", "$45.00", "3 unused subscriptions", "💡");
    StatCard *renewalCard = new StatCard("Upcoming Renewals", "4", "Next 30 days", "🔄");
    
    analyticsGrid->addWidget(avgCard, 0, 0);
    analyticsGrid->addWidget(savingsCard, 0, 1);
    analyticsGrid->addWidget(renewalCard, 0, 2);
    
    layout->addLayout(analyticsGrid);
    
    // Chart area
    QFrame *chartCard = createCard();
    QVBoxLayout *chartLayout = new QVBoxLayout(chartCard);
    
    QLabel *chartTitle = new QLabel("Spending Trends");
    chartTitle->setStyleSheet("color: #0f172a; font-size: 18px; font-weight: 600;");
    chartLayout->addWidget(chartTitle);
    
    QLabel *chartPlaceholder = new QLabel("📊 Monthly spending chart would appear here");
    chartPlaceholder->setStyleSheet("color: #94a3b8; font-size: 14px; padding: 60px;");
    chartPlaceholder->setAlignment(Qt::AlignCenter);
    chartLayout->addWidget(chartPlaceholder);
    
    layout->addWidget(chartCard);
    layout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    
    return page;
}

QWidget* MainWindow::createSubscriptionsPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8fafc;");
    
    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(20);
    layout->setContentsMargins(24, 24, 24, 24);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Video and Audio Setting");
    titleLabel->setStyleSheet("color: #0f172a; font-size: 24px; font-weight: 700;");
    
    QPushButton *filterBtn = new QPushButton("🔍 Filter");
    filterBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"
        "    color: #475569;"
        "    border: 2px solid #e2e8f0;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "    border-color: #cbd5e1;"
        "    background-color: #f8fafc;"
        "}"
    );
    filterBtn->setMaximumWidth(120);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(filterBtn);
    
    layout->addLayout(headerLayout);
    
    // Subscriptions list
    QFrame *listCard = createCard();
    QVBoxLayout *listLayout = new QVBoxLayout(listCard);
    listLayout->setSpacing(0);
    
    QStringList subscriptions = {
        "Netflix|$15.99|streaming",
        "Spotify|$9.99|streaming",
        "Adobe Creative Cloud|$54.99|productivity",
        "GitHub Pro|$4.00|development",
        "Amazon Prime|$14.99|shopping",
        "Apple iCloud+|$9.99|storage"
    };
    
    QVector<QColor> categoryColors = {
        QColor("#bef264"), QColor("#bef264"), QColor("#a78bfa"),
        QColor("#60a5fa"), QColor("#fdba74"), QColor("#94a3b8")
    };
    
    for (int i = 0; i < subscriptions.size(); ++i) {
        QStringList parts = subscriptions[i].split("|");
        listLayout->addWidget(new ActivityItem(
            parts[0], 
            "Active • Renews monthly", 
            parts[2], 
            parts[1], 
            categoryColors[i]
        ));
    }
    
    layout->addWidget(listCard);
    layout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    
    return page;
}

QWidget* MainWindow::createSettingsPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f8fafc;");
    
    QScrollArea *scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(20);
    layout->setContentsMargins(24, 24, 24, 24);
    
    QLabel *titleLabel = new QLabel("Settings & Preferences");
    titleLabel->setStyleSheet("color: #0f172a; font-size: 24px; font-weight: 700;");
    layout->addWidget(titleLabel);
    
    // Settings sections
    QFrame *settingsCard = createCard();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsCard);
    settingsLayout->setSpacing(16);
    
    QStringList settingsSections = {
        "⚙ General Settings",
        "🔔 Notifications & Alerts",
        "🎨 Appearance & Theme",
        "🔐 Privacy & Security",
        "💾 Backup & Export Data",
        "ℹ About & Support"
    };
    
    for (const QString &section : settingsSections) {
        QPushButton *sectionBtn = new QPushButton(section);
        sectionBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #f8fafc;"
            "    color: #334155;"
            "    border: none;"
            "    padding: 16px;"
            "    font-size: 15px;"
            "    font-weight: 500;"
            "    border-radius: 8px;"
            "    text-align: left;"
            "}"
            "QPushButton:hover {"
            "    background-color: #f1f5f9;"
            "}"
        );
        settingsLayout->addWidget(sectionBtn);
    }
    
    layout->addWidget(settingsCard);
    layout->addStretch();
    
    scrollArea->setWidget(scrollContent);
    
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    
    return page;
}

void MainWindow::setupPages()
{
    // Create main widget and layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create stacked widget
    stackedWidget = new QStackedWidget(this);
    
    // Create pages with modern SubTracker style
    page1 = createDashboardPage();
    page2 = createAnalyticsPage();
    page3 = createSubscriptionsPage();
    page4 = createSettingsPage();
    
    // Add pages to stacked widget
    stackedWidget->addWidget(page1);
    stackedWidget->addWidget(page2);
    stackedWidget->addWidget(page3);
    stackedWidget->addWidget(page4);
    
    mainLayout->addWidget(stackedWidget);
}

void MainWindow::setupNavigation()
{
    // Create navigation layout with modern style
    QWidget *navBar = new QWidget(this);
    navBar->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    navBar->setFixedHeight(40);
    
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(24, 8, 24, 8);
    
    // Previous button with icon
    prevButton = new QPushButton();
    prevButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-radius: 20px;"
        "}"
    );
    prevButton->setFixedSize(40, 40);
    prevButton->setIconSize(QSize(32, 32));
    
    // Next button with icon
    nextButton = new QPushButton();
    nextButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-radius: 20px;"
        "}"
    );
    nextButton->setFixedSize(40, 40);
    nextButton->setIconSize(QSize(32, 32));
    
    // Page indicator with dots
    QHBoxLayout *indicatorLayout = new QHBoxLayout();
    indicatorLayout->setSpacing(8);
    
    pageIndicator = new QLabel();
    pageIndicator->setAlignment(Qt::AlignCenter);
    
    // Create page dots
    for (int i = 0; i < 4; ++i) {
        QLabel *dot = new QLabel("●");
        dot->setStyleSheet(i == 0 ? 
            "color: #3b82f6; font-size: 12px;" : 
            "color: #cbd5e1; font-size: 12px;");
        dot->setObjectName(QString("dot_%1").arg(i));
        indicatorLayout->addWidget(dot);
    }
    
    // Page text
    pageIndicator->setText("");
    pageIndicator->setStyleSheet("color: #475569; font-size: 14px; font-weight: 600; margin: 0 12px;");
    pageIndicator->setVisible(false);
    
    // Add to navigation layout
    navLayout->addWidget(prevButton);
    navLayout->addStretch();
    navLayout->addLayout(indicatorLayout);
    navLayout->addStretch();
    navLayout->addWidget(nextButton);
    
    // Add navigation to main layout
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(navBar);
    }
    
    // Connect signals
    connect(prevButton, &QPushButton::clicked, this, &MainWindow::prevPage);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::nextPage);
    
    // Set initial state
    updateNavigation();
}

void MainWindow::nextPage()
{
    int currentIndex = stackedWidget->currentIndex();
    int nextIndex = (currentIndex + 1) % stackedWidget->count();
    animatePageTransition(nextIndex);
}

void MainWindow::prevPage()
{
    int currentIndex = stackedWidget->currentIndex();
    int prevIndex = (currentIndex - 1 + stackedWidget->count()) % stackedWidget->count();
    animatePageTransition(prevIndex);
}

void MainWindow::goToPage(int index)
{
    if (index >= 0 && index < stackedWidget->count()) {
        animatePageTransition(index);
    }
}

void MainWindow::animatePageTransition(int index)
{
    // Simple fade effect (you can enhance this with more complex animations)
    QWidget *currentWidget = stackedWidget->currentWidget();
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    currentWidget->setGraphicsEffect(effect);
    
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(200);
    animation->setStartValue(1.0);
    animation->setEndValue(0.3);
    
    connect(animation, &QPropertyAnimation::finished, [this, index, currentWidget]() {
        stackedWidget->setCurrentIndex(index);
        updateNavigation();
        
        // Fade back in
        QGraphicsOpacityEffect *effectIn = new QGraphicsOpacityEffect();
        stackedWidget->currentWidget()->setGraphicsEffect(effectIn);
        
        QPropertyAnimation *animationIn = new QPropertyAnimation(effectIn, "opacity");
        animationIn->setDuration(200);
        animationIn->setStartValue(0.3);
        animationIn->setEndValue(1.0);
        
        connect(animationIn, &QPropertyAnimation::finished, [this]() {
            stackedWidget->currentWidget()->setGraphicsEffect(nullptr);
        });
        
        animationIn->start(QPropertyAnimation::DeleteWhenStopped);
        
        // Clean up the old effect
        currentWidget->setGraphicsEffect(nullptr);
    });
    
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::updateNavigation()
{
    int currentIndex = stackedWidget->currentIndex();
    int totalPages = stackedWidget->count();
    
    // Update dots
    for (int i = 0; i < 4; ++i) {
        QLabel *dot = centralWidget()->findChild<QLabel*>(QString("dot_%1").arg(i));
        if (dot) {
            dot->setStyleSheet(i == currentIndex ? 
                "color: #3b82f6; font-size: 12px;" : 
                "color: #cbd5e1; font-size: 12px;");
        }
    }
    
    // Update button states and icons
    bool hasPrev = currentIndex > 0;
    bool hasNext = currentIndex < totalPages - 1;
    
    prevButton->setEnabled(hasPrev);
    nextButton->setEnabled(hasNext);
    
    // Set icons based on enabled state
    QPixmap prevPixmap(hasPrev ? ":/images/keyboard_arrow_left_128_black.png" : ":/images/keyboard_arrow_left_128_gray.png");
    if (!prevPixmap.isNull()) {
        prevButton->setIcon(QIcon(prevPixmap));
    }
    
    QPixmap nextPixmap(hasNext ? ":/images/keyboard_arrow_right_128_black.png" : ":/images/keyboard_arrow_right_128_gray.png");
    if (!nextPixmap.isNull()) {
        nextButton->setIcon(QIcon(nextPixmap));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    bool wasLandscape = isLandscape;
    isLandscape = width() > height();
    
    if (wasLandscape != isLandscape) {
        updateLayoutOrientation();
    }
}

void MainWindow::updateLayoutOrientation()
{
    // Update stat cards layout based on orientation
    if (statsLayout && !statCards.isEmpty()) {
        // Remove all widgets from layout
        while (statsLayout->count() > 0) {
            statsLayout->takeAt(0);
        }
        
        if (isLandscape) {
            // Landscape: 4 columns
            for (int i = 0; i < statCards.size(); ++i) {
                statsLayout->addWidget(statCards[i], 0, i);
                statCards[i]->updateOrientation(true);
            }
        } else {
            // Portrait: 2 columns
            for (int i = 0; i < statCards.size(); ++i) {
                statsLayout->addWidget(statCards[i], i / 2, i % 2);
                statCards[i]->updateOrientation(false);
            }
        }
    }
}

void MainWindow::toggleResourceMonitor()
{
    resourceMonitorVisible = !resourceMonitorVisible;
    
    if (resourceMonitorVisible) {
        resourceGraphWidget->show();
        toggleResourceBtn->setText("▼ Hide");
    } else {
        resourceGraphWidget->hide();
        toggleResourceBtn->setText("▶ Show");
    }
}

void MainWindow::onWifiButtonClicked()
{
    qDebug() << "WiFi button clicked";
    
    if (!m_ethernet || !m_ethernet->isConnected()) {
        QMessageBox::warning(this, "Connection Error", 
            "Not connected to robot computer.\nPlease check ethernet connection to your_static_eth_ip:8888");
        return;
    }
    
    // Request WiFi list from robot
    m_ethernet->requestWifiList();
}

void MainWindow::onWifiListReceived(const QJsonArray &wifiList)
{
    qDebug() << "Received WiFi list with" << wifiList.size() << "networks";
    
    // Create dialog to show WiFi list
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Available WiFi Networks");
    
    // Set larger size for Android tablets (1280x800)
    // Use 90% of screen height and 80% of screen width
    QSize screenSize = QApplication::primaryScreen()->size();
    int dialogWidth = qMin(700, static_cast<int>(screenSize.width() * 0.8));
    int dialogHeight = qMin(650, static_cast<int>(screenSize.height() * 0.9));
    dialog->resize(dialogWidth, dialogHeight);
    
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    
    QLabel *titleLabel = new QLabel("Select a WiFi network:");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; margin-bottom: 8px;");
    layout->addWidget(titleLabel);
    
    // Create list widget with explicit size policy
    QListWidget *listWidget = new QListWidget();
    listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    listWidget->setMinimumHeight(400);
    
    listWidget->setStyleSheet(
        "QListWidget {"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    background-color: white;"
        "}"
        "QListWidget::item {"
        "    padding: 16px 12px;"
        "    border-bottom: 1px solid #f1f5f9;"
        "    min-height: 48px;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #f8fafc;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #dbeafe;"
        "    color: #1e40af;"
        "}"
    );
    
    // Add WiFi networks to list
    for (int i = 0; i < wifiList.size(); ++i) {
        QJsonObject network = wifiList[i].toObject();
        QString ssid = network["ssid"].toString();
        QString security = network["security"].toString();
        
        // Format: SSID (Security Type)
        QString displayText = QString("%1  🔒 %2").arg(ssid, security);
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, ssid); // Store SSID in user data
        item->setData(Qt::UserRole + 1, security); // Store security type
        listWidget->addItem(item);
    }
    
    layout->addWidget(listWidget);
    
    // Add buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *connectBtn = new QPushButton("Connect");
    connectBtn->setMinimumHeight(48);
    connectBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "    border: none;"
        "    padding: 14px 28px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2563eb;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cbd5e1;"
        "}"
    );
    connectBtn->setEnabled(false);
    
    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setMinimumHeight(48);
    cancelBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9;"
        "    color: #475569;"
        "    border: 1px solid #e2e8f0;"
        "    padding: 14px 28px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e2e8f0;"
        "}"
    );
    
    // Enable connect button when item is selected
    connect(listWidget, &QListWidget::itemSelectionChanged, [connectBtn, listWidget]() {
        connectBtn->setEnabled(listWidget->currentItem() != nullptr);
    });
    
    // Connect button clicked - check security and ask for password if needed
    connect(connectBtn, &QPushButton::clicked, [this, listWidget, dialog, wifiList]() {
        QListWidgetItem *item = listWidget->currentItem();
        if (item) {
            QString ssid = item->data(Qt::UserRole).toString();
            QString security = item->data(Qt::UserRole + 1).toString();

            QString password = "";

            // Check if password is needed (not an open network)
            if (security != "Open" && security != "--") {
                // Create a small custom password dialog with show/hide toggle
                QDialog *pwdDialog = new QDialog(dialog);
                pwdDialog->setWindowTitle("WiFi Password");
                pwdDialog->setModal(true);

                QVBoxLayout *pwdLayout = new QVBoxLayout(pwdDialog);
                QLabel *prompt = new QLabel(QString("Enter password for '%1':").arg(ssid));
                prompt->setStyleSheet("font-size:13px; margin-bottom:6px;");
                pwdLayout->addWidget(prompt);

                QHBoxLayout *hLayout = new QHBoxLayout();
                QLineEdit *pwdEdit = new QLineEdit();
                pwdEdit->setEchoMode(QLineEdit::Password);
                pwdEdit->setMinimumWidth(300);
                pwdEdit->setPlaceholderText("Password");

                QToolButton *toggleBtn = new QToolButton();
                toggleBtn->setCheckable(true);
                toggleBtn->setText("Show");
                toggleBtn->setFixedSize(48, 24);

                connect(toggleBtn, &QToolButton::toggled, pwdEdit, [pwdEdit, toggleBtn](bool checked){
                    if (checked) {
                        pwdEdit->setEchoMode(QLineEdit::Normal);
                        toggleBtn->setText("Hide");
                    } else {
                        pwdEdit->setEchoMode(QLineEdit::Password);
                        toggleBtn->setText("Show");
                    }
                });

                hLayout->addWidget(pwdEdit);
                hLayout->addWidget(toggleBtn);
                pwdLayout->addLayout(hLayout);

                QHBoxLayout *btns = new QHBoxLayout();
                QPushButton *cancelPwd = new QPushButton("Cancel");
                QPushButton *okPwd = new QPushButton("Connect");
                btns->addStretch();
                btns->addWidget(cancelPwd);
                btns->addWidget(okPwd);
                pwdLayout->addLayout(btns);

                connect(cancelPwd, &QPushButton::clicked, pwdDialog, &QDialog::reject);
                connect(okPwd, &QPushButton::clicked, pwdDialog, &QDialog::accept);

                if (pwdDialog->exec() != QDialog::Accepted) {
                    pwdDialog->deleteLater();
                    return; // user cancelled
                }

                password = pwdEdit->text();
                pwdDialog->deleteLater();
            }

            // Connect to WiFi
            if (m_ethernet) {
                m_ethernet->connectToWifi(ssid, password);
            }

            dialog->accept();
        }
    });
    
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(connectBtn);
    
    layout->addLayout(buttonLayout);
    
    dialog->exec();

    dialog->deleteLater();
}

void MainWindow::onWifiStatusReceived(const QJsonObject &status)
{
    qDebug() << "Received WiFi status:" << status;
    
    bool connected = status["connected"].toBool();
    QString ssid = status["ssid"].toString();
    QString ip = status["ip"].toString();
    QString robotName = status["robot_name"].toString();
    
    // Update WiFi status card
    if (wifiStatusCard) {
        wifiStatusCard->updateWifiStatus(connected, ssid, ip, robotName);
    }
}

void MainWindow::onWifiConnecting(const QString &ssid)
{
    qDebug() << "Connecting to WiFi:" << ssid;
    
    // Update card to show connecting status with animation
    if (wifiStatusCard) {
        wifiStatusCard->showConnecting();
    }
}

void MainWindow::onWifiConnectionResult(bool success, const QString &message)
{
    qDebug() << "WiFi connection result:" << success << message;
    
    if (success) {
        // Request updated WiFi status after a short delay to allow connection to stabilize
        if (m_ethernet && m_ethernet->isConnected()) {
            QTimer::singleShot(1000, this, [this]() {
                if (m_ethernet && m_ethernet->isConnected()) {
                    m_ethernet->requestWifiStatus();
                }
            });
        }
    } else {
        // Show failed status
        if (wifiStatusCard) {
            wifiStatusCard->showConnectionFailed();
        }
        QMessageBox::warning(this, "WiFi Connection Failed", message);
    }
}

void MainWindow::onEthernetError(const QString &errorMessage)
{
    qWarning() << "Ethernet error:" << errorMessage;
    
    // Don't show error dialog on startup if connection fails
    // User will see "Disconnected" status in the WiFi card
    if (errorMessage.contains("get_wifi_status") || errorMessage.contains("Unknown command")) {
        // This is a command error, show it
        QMessageBox::warning(this, "Network Error", 
            QString("Connection error:\n%1").arg(errorMessage));
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QMainWindow::event(event);
}

bool MainWindow::gestureEvent(QGestureEvent *event)
{
    if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) {
        swipeTriggered(static_cast<QSwipeGesture*>(swipe));
    }
    return true;
}

void MainWindow::swipeTriggered(QSwipeGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        if (gesture->horizontalDirection() == QSwipeGesture::Left) {
            nextPage();
        } else if (gesture->horizontalDirection() == QSwipeGesture::Right) {
            prevPage();
        }
    }
}
