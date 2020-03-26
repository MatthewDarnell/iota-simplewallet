#include "importaccountdialog.h"
#include <forms/ui_importaccountdialog.h>

#include <guiconstants.h>
#include <walletmodel.h>

#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>


ImportAccountDialog::ImportAccountDialog(Mode _mode, QWidget *parent,
                                         SecureString* username_out,
                                         SecureString* passphrase_out,
                                         SecureString* seed_out) :
    QDialog(parent),
    ui(new Ui::ImportAccountDialog),
    mode(_mode),
    fCapsLock(false),
    m_username_out(username_out),
    m_passphrase_out(passphrase_out),
    m_seed_out(seed_out)
{
    ui->setupUi(this);

    // support only seed for now
    Q_ASSERT(m_username_out);
    Q_ASSERT(m_passphrase_out);

    if (mode == Mode::Seed) {
        Q_ASSERT(m_seed_out);
    }

    ui->usernameEdit->setMinimumSize(ui->usernameEdit->sizeHint());
    ui->passEdit->setMinimumSize(ui->passEdit->sizeHint());
    ui->seedEdit->setMinimumSize(ui->seedEdit->sizeHint());

    ui->usernameEdit->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->passEdit->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->seedEdit->setMaxLength(MAX_PASSPHRASE_SIZE);

    // Setup Caps Lock detection.
    ui->usernameEdit->installEventFilter(this);
    ui->passEdit->installEventFilter(this);
    ui->seedEdit->installEventFilter(this);

    if (mode == Mode::File) {
        ui->seedLabel->setVisible(false);
        ui->seedEdit->setVisible(false);
    }

    textChanged();
    connect(ui->toggleShowPasswordButton, &QPushButton::toggled, this, &ImportAccountDialog::toggleShowPassword);
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &ImportAccountDialog::textChanged);
    connect(ui->passEdit, &QLineEdit::textChanged, this, &ImportAccountDialog::textChanged);
    connect(ui->seedEdit, &QLineEdit::textChanged, this, &ImportAccountDialog::textChanged);

    setTabOrder(ui->usernameEdit, ui->passEdit);
    setTabOrder(ui->passEdit, ui->seedEdit);
}

ImportAccountDialog::~ImportAccountDialog()
{
    secureClearPassFields();
    delete ui;
}

void ImportAccountDialog::accept()
{
    SecureString username, password, seed;
    username.reserve(MAX_PASSPHRASE_SIZE);
    password.reserve(MAX_PASSPHRASE_SIZE);
    seed.reserve(MAX_PASSPHRASE_SIZE);

    username.assign(ui->usernameEdit->text().toStdString().c_str());
    password.assign(ui->passEdit->text().toStdString().c_str());
    seed.assign(ui->seedEdit->text().toStdString().c_str());

    secureClearPassFields();

    m_username_out->assign(username);
    m_passphrase_out->assign(password);
    if (mode == Mode::Seed) {
        m_seed_out->assign(seed);
    }

    QDialog::accept();
}

void ImportAccountDialog::textChanged()
{
    // Validate input, set Ok button to enabled when acceptable
    bool acceptable = false;
    switch(mode)
    {
    case Mode::File: // New passphrase x2
        acceptable = !ui->passEdit->text().isEmpty() && !ui->usernameEdit->text().isEmpty();
        break;
    case Mode::Seed: // Old passphrase x1, new passphrase x2
        acceptable = !ui->passEdit->text().isEmpty() && !ui->usernameEdit->text().isEmpty() && !ui->seedEdit->text().isEmpty();
        break;
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptable);
}

static void SecureClearQLineEdit(QLineEdit* edit)
{
    // Attempt to overwrite text so that they do not linger around in memory
    edit->setText(QString(" ").repeated(edit->text().size()));
    edit->clear();
}

void ImportAccountDialog::secureClearPassFields()
{
    SecureClearQLineEdit(ui->usernameEdit);
    SecureClearQLineEdit(ui->passEdit);
    SecureClearQLineEdit(ui->seedEdit);
}

void ImportAccountDialog::toggleShowPassword(bool show)
{
    ui->toggleShowPasswordButton->setDown(show);
    const auto mode = show ? QLineEdit::Normal : QLineEdit::Password;
    ui->passEdit->setEchoMode(mode);
}

bool ImportAccountDialog::event(QEvent *event)
{
    // Detect Caps Lock key press.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_CapsLock) {
            fCapsLock = !fCapsLock;
        }
        if (fCapsLock) {
            ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
        } else {
            ui->capsLabel->clear();
        }
    }
    return QWidget::event(event);
}

bool ImportAccountDialog::eventFilter(QObject *object, QEvent *event)
{
    /* Detect Caps Lock.
     * There is no good OS-independent way to check a key state in Qt, but we
     * can detect Caps Lock by checking for the following condition:
     * Shift key is down and the result is a lower case character, or
     * Shift key is not down and the result is an upper case character.
     */
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QString str = ke->text();
        if (str.length() != 0) {
            const QChar *psz = str.unicode();
            bool fShift = (ke->modifiers() & Qt::ShiftModifier) != 0;
            if ((fShift && *psz >= 'a' && *psz <= 'z') || (!fShift && *psz >= 'A' && *psz <= 'Z')) {
                fCapsLock = true;
                ui->capsLabel->setText(tr("Warning: The Caps Lock key is on!"));
            } else if (psz->isLetter()) {
                fCapsLock = false;
                ui->capsLabel->clear();
            }
        }
    }
    return QDialog::eventFilter(object, event);
}
