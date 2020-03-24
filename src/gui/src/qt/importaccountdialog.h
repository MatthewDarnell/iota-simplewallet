#ifndef IMPORTACCOUNTDIALOG_H
#define IMPORTACCOUNTDIALOG_H

#include <QDialog>

#include <support/allocators/secure.h>

class WalletModel;

namespace Ui {
class ImportAccountDialog;
}

class ImportAccountDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode {
        File,
        Seed,
    };

    explicit ImportAccountDialog(Mode _mode, QWidget *parent,
                                 SecureString* username_out,
                                 SecureString* passphrase_out,
                                 SecureString* seed_out);
    ~ImportAccountDialog();

    void accept();

private Q_SLOTS:
    void textChanged();
    void secureClearPassFields();
    void toggleShowPassword(bool);

protected:
    bool event(QEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::ImportAccountDialog *ui;
    Mode mode;
    bool fCapsLock;
    SecureString* m_username_out { nullptr };
    SecureString* m_passphrase_out { nullptr };
    SecureString* m_seed_out { nullptr };
};

#endif // IMPORTACCOUNTDIALOG_H
