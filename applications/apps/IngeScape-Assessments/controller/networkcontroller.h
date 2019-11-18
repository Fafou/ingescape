/*
 *	IngeScape Assessments
 *
 *  Copyright © 2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include "I2PropertyHelpers.h"
#include <controller/ingescapenetworkcontroller.h>


/**
 * @brief The NetworkController class defines the controller for network communications
 */
class NetworkController : public IngeScapeNetworkController
{
    Q_OBJECT


public:

    /**
     * @brief Constructor
     * @param parent
     */
    explicit NetworkController(QObject *parent = nullptr);


    /**
      * @brief Destructor
      */
    ~NetworkController() Q_DECL_OVERRIDE;


    /**
     * @brief Manage a "Shouted" message
     * @param peerId
     * @param peerName
     * @param zMessage
     */
    //void manageShoutedMessage(QString peerId, QString peerName, zmsg_t* zMessage) Q_DECL_OVERRIDE;


    /**
     * @brief Manage a "Whispered" message
     * @param peerId
     * @param peerName
     * @param zMessage
     */
    void manageWhisperedMessage(QString peerId, QString peerName, zmsg_t* zMessage) Q_DECL_OVERRIDE;



Q_SIGNALS:


public Q_SLOTS:


private:


};

QML_DECLARE_TYPE(NetworkController)

#endif // NETWORKCONTROLLER_H
