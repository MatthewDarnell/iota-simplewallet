// Copyright (c) 2011-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_RPCCONSOLE_H
#define BITCOIN_QT_RPCCONSOLE_H

#include <guiutil.h>

#include <QWidget>
#include <QCompleter>
#include <QThread>


class ClientModel;
class PlatformStyle;
class RPCTimerInterface;
class WalletModel;

namespace interfaces {
    class Node;
}

namespace Ui {
    class RPCConsole;
}

QT_BEGIN_NAMESPACE
class QMenu;
class QItemSelection;
QT_END_NAMESPACE

/** Local Bitcoin RPC console. */
class RPCConsole: public QWidget
{
    Q_OBJECT

public:
    explicit RPCConsole(interfaces::Node& node, const PlatformStyle *platformStyle, QWidget *parent);
    ~RPCConsole();

    static bool RPCParseCommandLine(interfaces::Node* node, std::string &strResult, const std::string &strCommand, bool fExecute, std::string * const pstrFilteredOut = nullptr, const WalletModel* wallet_model = nullptr);
    static bool RPCExecuteCommandLine(interfaces::Node& node, std::string &strResult, const std::string &strCommand, std::string * const pstrFilteredOut = nullptr, const WalletModel* wallet_model = nullptr) {
        return RPCParseCommandLine(&node, strResult, strCommand, true, pstrFilteredOut, wallet_model);
    }

    void setClientModel(ClientModel *model);
    void addWallet(WalletModel * const walletModel);
    void removeWallet(WalletModel* const walletModel);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

    enum class TabTypes {
        INFO,
        CONSOLE
    };

    std::vector<TabTypes> tabs() const { return {TabTypes::INFO, TabTypes::CONSOLE}; }

    QString tabTitle(TabTypes tab_type) const;
    QKeySequence tabShortcut(TabTypes tab_type) const;

protected:
    virtual bool eventFilter(QObject* obj, QEvent *event);
    void keyPressEvent(QKeyEvent *);

private Q_SLOTS:
    void on_lineEdit_returnPressed();
    void on_tabWidget_currentChanged(int index);
    /** open the debug.log from the current datadir */
    void on_openDebugLogfileButton_clicked();
    /** update traffic statistics */
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);


public Q_SLOTS:
    void clear(bool clearHistory = true);
    void fontBigger();
    void fontSmaller();
    void setFontSize(int newSize);
    /** Append the message to the message widget */
    void message(int category, const QString &msg) { message(category, msg, false); }
    void message(int category, const QString &message, bool html);
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set network state shown in the UI */
    void setNetworkActive(bool networkActive);
    /** Set number of blocks and last block date shown in the UI */
    void setNumBlocks(int solidCount, int count, QString blockTipSolid, QString blockTip);
    /** Go forward or back in history */
    void browseHistory(int offset);
    /** Scroll console view to end */
    void scrollToEnd();
    /** set which tab has the focus (is visible) */
    void setTabFocus(enum TabTypes tabType);

    /** Set app info shown in the UI */
    void setAppInfo(QString appName, QString appVersion, QString connectedNode);

    void updateClientVersion(const QString &version);

Q_SIGNALS:
    // For RPC command executor
    void cmdRequest(const QString &command, const WalletModel* wallet_model);

private:
    void startExecutor();
    /** show detailed information on ui about selected node */
//    void updateNodeDetail(const CNodeCombinedStats *stats);

    enum ColumnWidths
    {
        ADDRESS_COLUMN_WIDTH = 200,
        SUBVERSION_COLUMN_WIDTH = 150,
        PING_COLUMN_WIDTH = 80,
        BANSUBNET_COLUMN_WIDTH = 200,
        BANTIME_COLUMN_WIDTH = 250

    };

    interfaces::Node& m_node;
    Ui::RPCConsole* const ui;
    ClientModel *clientModel = nullptr;
    QStringList history;
    int historyPtr = 0;
    QString cmdBeforeBrowsing;
    const PlatformStyle* const platformStyle;
    RPCTimerInterface *rpcTimerInterface = nullptr;
    int consoleFontSize = 0;
    QCompleter *autoCompleter = nullptr;
    QThread thread;
    WalletModel* m_last_wallet_model{nullptr};

    /** Update UI with latest network info from model. */
    void updateNetworkState();

private Q_SLOTS:
    void updateAlerts(const QString& warnings);
};

//#endif

#endif // BITCOIN_QT_RPCCONSOLE_H
