#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setStyleForm();
    signal_PushButtons();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setStyleForm()
{
    layout = new QVBoxLayout(this);

    data_label = new QLabel("Enter the data");
    data_edit = new QLineEdit;
    layout->addWidget(data_label);
    layout->addWidget(data_edit);

    key_label = new QLabel("Enter the key");
    key_edit = new QLineEdit;
    layout->addWidget(key_label);
    layout->addWidget(key_edit);

    encrypt_button = new QPushButton("Encrypt");
    layout->addWidget(encrypt_button);


    encrypted_label = new QLabel ("Encrypted data:");
    encrypted_edit = new QTextEdit;
    encrypted_edit->setReadOnly(true);
    layout->addWidget(encrypted_label);
    layout->addWidget(encrypted_edit);

    decrypt_button = new QPushButton("Decrypt");
    layout->addWidget(decrypt_button);

    decrypted_label = new QLabel ("Decrypted data:");
    decrypted_edit = new QTextEdit;
    decrypted_edit->setReadOnly(true);
    layout->addWidget(decrypted_label);
    layout->addWidget(decrypted_edit);

}

void Widget::RC4(QByteArray &data, int data_length, QByteArray &key, int key_length)
{
    const int SIZE = 256;
    QVector<int> S(SIZE);
    int i, j;

    for (int i = 0; i < SIZE; i++)
        S[i] = i;

    j = 0;
    for (int i = 0; i < SIZE; i++)
    {
        j = (j + S[i] + key[i % key_length]) % SIZE;
        qSwap(S[i], S[j]);
    }

    i = j = 0;
    for (int k = 0; k < data_length; k++)
    {
        i = (i + 1) % SIZE;
        j = (j + S[i]) % SIZE;
        qSwap(S[i], S[j]);
        int t = (S[i] + S[j]) % SIZE;
        data[k] ^= S[t];
    }
}

void Widget::signal_PushButtons()
{
    QObject::connect(encrypt_button,&QPushButton::clicked,this,&Widget::Encryption);
    QObject::connect(decrypt_button,&QPushButton::clicked,this,&Widget::Decryption);
}

void Widget::Encryption()
{
    QByteArray data = data_edit->text().toUtf8();
    QByteArray key = key_edit->text().toUtf8();

    RC4(data, data.size(), key, key.size());

    encrypted_edit->setText(QString::fromUtf8(data.toBase64()));
}

void Widget::Decryption()
{
    QByteArray encrypted_data = QByteArray::fromBase64(encrypted_edit->toPlainText().toUtf8());
    QByteArray key = key_edit->text().toUtf8();

    RC4(encrypted_data, encrypted_data.size(), key, key.size());

    decrypted_edit->setText(QString::fromUtf8(encrypted_data));
}


