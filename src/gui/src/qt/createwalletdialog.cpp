// Copyright (c) 2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)

#endif

#include <createwalletdialog.h>
#include <forms/ui_createwalletdialog.h>

#include <QPushButton>

CreateWalletDialog::CreateWalletDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::CreateWalletDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->wallet_name_line_edit->setFocus(Qt::ActiveWindowFocusReason);

    connect(ui->wallet_name_line_edit, &QLineEdit::textEdited, [this](const QString& text) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
    });
}

CreateWalletDialog::~CreateWalletDialog()
{
    delete ui;
}

QString CreateWalletDialog::walletName() const
{
    return ui->wallet_name_line_edit->text();
}

bool CreateWalletDialog::isEncryptWalletChecked() const
{
    return true;
}
