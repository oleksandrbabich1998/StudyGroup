#include "startpage.h"
#include "ui_startpage.h"
#include "server.h"
#include "joingrouppage.h"
#include <QMessageBox>

StartPage::StartPage(QString name, QWidget *parent) :
    SGWidget(name, parent),
    ui(new Ui::StartPage)
{
    ui->setupUi(this);

    QPixmap logo(":/resources/img/GSLogoName1.png");    // StudyGroup logo
    ui->label_logo->setPixmap(logo.scaled(250,300,Qt::KeepAspectRatio,Qt::SmoothTransformation));     // Resize to fit

    // check/X icons are hidden initially
    ui->label_username_check->hide();
    ui->label_password1_check->hide();
    ui->label_password2_check->hide();
    ui->label_email_check->hide();

    ui->tabWidget->setCurrentWidget(ui->tab_sign_in);

    // Account Security
    recover = new AccountSecurity();
    ui->recover_account->addWidget(recover);
    connect(recover, SIGNAL(expand_tabwidget()), this, SLOT(expand_tabwidget()));
}

StartPage::~StartPage()
{
    delete ui;
}
void StartPage::do_work(){

}

void StartPage::on_signin_button_clicked()
{
    QString email;  // Return parameter from the server response

    server::test("startpage", "this is from the signin slot");
    QString full_string = server::LOGIN + ui->lineEdit_username->text() + " " + ui->lineEdit_password->text();

    if(server::request_response(full_string, email))
    {
        // Now logged in!
        ui->lineEdit_username->setText("");
        ui->lineEdit_password->setText("");
        qDebug() << "Email:" << email;
        // Set username and password
        //user_info->setUsername(username);                     // FIX THESE WHEN USER CLASS IS DONE
        //user_info->setPassword(password);
        // Update settings page
        //ui->settings_email->setText(email);                   // FIX THESE WHEN SETTINGS CLASS IS DONE
        //ui->settings_username->setText(user_info->getUsername());

        emit logged_in(0); // Change main page
    }
}

void StartPage::on_singup_button_clicked()
{
    QString full_string = server::CREATE_ACCOUNT + ui->lineEdit_email->text() +
            " " + ui->lineEdit_username_signup->text() +
            " " + ui->lineEdit_password2->text();
    QString response;
    if(server::request_response(full_string, response))
    {
        QMessageBox success_box;
        success_box.setText(response);
        success_box.exec();
        ui->lineEdit_email->setText("");
        ui->lineEdit_username_signup->setText("");
        ui->lineEdit_password1->setText("");
        ui->lineEdit_password2->setText("");
    }
}

/*****************************************************
 * ACCOUNT SIGNUP
 */

void StartPage::on_lineEdit_email_textChanged(const QString &email)
{
    if (email == "") {
        ui->label_email_check->hide();
    } else {
        QString error_msg;
        bool valid = true; //user_info->usernameValidation(username,  error_msg);
        set_valid_icons(ui->label_email_check, ui->lineEdit_email, error_msg, valid);
    }
}

void StartPage::on_lineEdit_username_signup_textChanged(const QString &username)
{
    if (username == "") {
        ui->label_username_check->hide();
    } else {
        QString error_msg;
        bool valid = true; //user_info->usernameValidation(username,  error_msg);
        set_valid_icons(ui->label_username_check, ui->lineEdit_username_signup, error_msg, valid);
    }
}

void StartPage::on_lineEdit_password1_textChanged(const QString &password1)
{
    if (password1 == "") {
        ui->label_password1_check->hide();
    } else {
        QString error_msg;
        bool valid = true; //user_info->usernameValidation(username,  error_msg);
        set_valid_icons(ui->label_password1_check, ui->lineEdit_password1, error_msg, valid);
    }
}

void StartPage::on_lineEdit_password2_textChanged(const QString &password2)
{
    if (password2 == "") {
        ui->label_password2_check->hide();
    } else {
        QString error_msg;
        bool valid = true; //user_info->usernameValidation(username,  error_msg);
        set_valid_icons(ui->label_password2_check, ui->lineEdit_password2, error_msg, valid);
    }
}

void StartPage::set_valid_icons(QLabel* this_label, QLineEdit* this_line, QString error_msg, bool valid){
    QPixmap mark = valid ? QPixmap(":/resources/img/check_mark.png") : QPixmap(":/resources/img/x_mark.png");
    this_label->setPixmap(mark.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    this_label->show();
}

/*****************************************************************
 * ACCOUNT RECOVERY
 */

void StartPage::on_pushButton_recover_pass_clicked()
{
    recover->display_recovery_page(1);
    ui->recover_account->setCurrentWidget(recover);
    /*QString username = ui->lineEdit_recover_pass_1->text();
    QString email = ui->lineEdit_recover_pass_2->text();*/

    ui->tab_recover_account->layout()->setContentsMargins(0,0,0,0);
    //ui->tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

}

void StartPage::on_pushButton_recover_user_clicked()
{
    recover->display_recovery_page(0);
    ui->recover_account->setCurrentWidget(recover);
}

void StartPage::on_tabWidget_tabBarClicked(int index)
{
    recover->clear_text();
    ui->recover_account->setCurrentWidget(ui->recover_acct_page);
    ui->spacer_top->changeSize(0,25,QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    ui->spacer_bottom->changeSize(0,50,QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
}
void StartPage::expand_tabwidget(){
    qDebug() << "EXPANDING WIDGET";
    ui->spacer_top->changeSize(0,100,QSizePolicy::Fixed, QSizePolicy::Minimum);
    ui->spacer_bottom->changeSize(0,100,QSizePolicy::Fixed, QSizePolicy::Minimum);

}


