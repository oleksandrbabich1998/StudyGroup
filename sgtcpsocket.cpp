#include "sgtcpsocket.h"
#include <QMessageBox>
#include <QDateTime>
#include <QNetworkDatagram>

SGTCPSocket::SGTCPSocket(QObject *parent) : QObject(parent)
{
    my_tcp_socket = new QTcpSocket();
    connect(my_tcp_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(reconnect_socket(QAbstractSocket::SocketState)));
    connect(my_tcp_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    connect(my_tcp_socket, SIGNAL(readyRead()), this, SLOT(read_socket_send_signal()));

    success_flag = false;
    fail_flag = false;
    success_message = nullptr;
    reconnecting = false;
}

SGTCPSocket::SGTCPSocket(const SGTCPSocket &sock) : QObject()
{
    qDebug() << "UHHH";
}

SGTCPSocket::~SGTCPSocket()
{
    my_tcp_socket->deleteLater();
}

void SGTCPSocket::connect_server()
{
    my_tcp_socket->connectToHost("52.14.84.3", 9001); // CSCI 150 SERVER
    // Connect the socket to the host
    //my_tcp_socket->connectToHost("52.14.84.3", 9001); // CSCI 150 SERVER
    //my_tcp_socket->connectToHost("localhost", 9001);
    // If it ever disconnects (including while trying this), the socket will
    // continuouslyq try to reconnect. See reconnect_socket().
}

void SGTCPSocket::write(QString message)
{
    QString message_length = QString::number(message.size());
    message = message.prepend(message_length.rightJustified(5, '0', true));
    my_tcp_socket->write(message.toLatin1());
}

void SGTCPSocket::write(QByteArray message)
{
    QByteArray message_length = QByteArray::number(message.size());
    message = message.prepend(message_length.rightJustified(5, '0', true));
    my_tcp_socket->write(message);
}

void SGTCPSocket::error(QAbstractSocket::SocketError err)
{
    qDebug() << my_tcp_socket->errorString();
}

void SGTCPSocket::reconnect_socket(QAbstractSocket::SocketState current_state)
{
    qDebug() << current_state;

    if(QAbstractSocket::UnconnectedState == current_state)
    {
        // Here because connection with the server was severed
        // Either by server going offline or client internet going out
        // So, try to continually reconnect until successful.
        if(reconnecting == false) {
            // First time we are trying to reconnect
            emit disconnected();
            // Show error message only initally
            QMessageBox reconnect_box;
            reconnect_box.setText("No internet connection to server.");
            reconnect_box.setIcon(QMessageBox::Warning);
            reconnect_box.exec();
        }
        reconnecting = true;

        qDebug() << "Trying to reconnect...";
        my_tcp_socket->connectToHost("52.14.84.3", 9001); // CSCI 150 SERVER
    }

    if(QAbstractSocket::ConnectedState == current_state)
    {
        if(reconnecting == true) {
            QMessageBox connected_box;
            connected_box.setText("Connection to server has been reestablished.");
            connected_box.exec();
            reconnecting = false;
        }
        emit connected();
    }
}

bool SGTCPSocket::read_socket_helper(QString& out_message)
{
    qDebug() << "Sending info...";
    success_flag = false;
    fail_flag = false;
    success_message = nullptr;

tryread:
    if((QAbstractSocket::ConnectedState == my_tcp_socket->state()) && (my_tcp_socket->waitForReadyRead(5000)))
    {
        if(!(success_flag || fail_flag))
            goto tryread;
        if(success_flag)
        {
            out_message = success_message;
            return true;
        }

        if(fail_flag)
        {
            return false; // Wrong info
        }
    } else {
        QMessageBox timeout_box;
        timeout_box.setText("Network Operation Timeout");
        timeout_box.setInformativeText("Either you aren't connected to the internet, or the server is down.");
        timeout_box.setIcon(QMessageBox::Warning);
        timeout_box.exec();
        return false; // Timeout
    }
}

QByteArray SGTCPSocket::single_message()
{
    qDebug() << "Receiving info...";
    QString message_size_str = my_tcp_socket->read(5);  // Read first 5 bytes, which is the serialized message size
    qDebug() << "Message size: " << message_size_str;

    int message_size = message_size_str.toInt();    // Convert the size to an integer
    while((my_tcp_socket->bytesAvailable() < message_size) && (my_tcp_socket->waitForReadyRead())) {
        qDebug() << "Message not here yet... bytes: " << my_tcp_socket->bytesAvailable();
        my_tcp_socket->waitForBytesWritten();
        // Do nothing until the right amount is available!
    }
    return my_tcp_socket->read(message_size); // Read the message, which is the next size bytes
}

QString SGTCPSocket::get_object_name(QByteArray &message)
{
    QString message_string(message);
    QString first_section = message_string.section(' ', 0, 0);
    QString code = first_section.left(4);  // Get the code
    first_section.remove(0, 4);  // Remove the code
    if ((code == "USCH") || (code == "NUSR") || (code == "NCHT") || (code == "NWFG") || (code == "RMFG"))
    {
        message.remove(4, first_section.length());
        return first_section;
    }
    else if((code == "WBLN") || (code == "NUWB") || (code == "WBUP"))
    {
        message.remove(4, first_section.length());
        return first_section + " whiteboard";
    }
    else if((code == "FCFT") || (code == "FCBK"))
    {
        message.remove(4, first_section.length());
        return first_section + " flashcard";
    }
    else if((code == "UPRG") || (code == "UPFG"))
    {
        return "homepage";
    }
    else if(code == "RSLT")
    {
        return "socialarea";
    }
    else if(code == "REQQ")
    {
        qDebug() << "*** REQQ ***";
        return "create account";
    }
    else if (code == "RUSR")
    {
        message.remove(4, first_section.length());
        return "recover username";
    }
    else if(code == "RPWD")
    {
        qDebug() << "Recover password message";
        return "reset password";
    }
}

void SGTCPSocket::read_socket_send_signal()
{
    while(my_tcp_socket->bytesAvailable() >= 5)
    {
        QByteArray message_ba = single_message();

        //qDebug() << "Server code: " << message_ba.left(4);

        // First short circuit in case its just a succ or fail message
        if (QString(message_ba).left(4) == "SUCC")
        {
            qDebug() << "SUCCESSSSSSS";
            // Set the success flag and message
            success_flag = true;
            message_ba.remove(0, 4);
            success_message = message_ba;
            //qDebug() << "Server message: " << success_message;
        }
        else if (QString(message_ba).left(4) == "FAIL")
        {
            // Set the fail flag
            fail_flag = true;
            message_ba.remove(0, 4);
            // Display the failure message to user
            if (message_ba.size() > 0) {
                QMessageBox timeout_box;
                timeout_box.setText("Error");
                timeout_box.setInformativeText(message_ba);
                timeout_box.setIcon(QMessageBox::Warning);
                timeout_box.exec();
            }
        }
        else {
            QString object_name = get_object_name(message_ba);
            emit new_message(object_name, message_ba);
        }
    }

    return;
}
