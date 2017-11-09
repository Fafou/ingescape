
/*
 *	IOPValueEffectM
 *
 *  Copyright (c) 2016-2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *
 */

#ifndef IOPVALUEEFFECTM_H
#define IOPVALUEEFFECTM_H

#include <QObject>
#include <QtQml>
#include <QQmlEngine>
#include <QJSEngine>

#include "I2PropertyHelpers.h"

#include "model/iop/agentiopm.h"
#include "model/scenario/actioneffectm.h"




/**
 * @brief The IOPValueEffectM class defines the main controller of our application
 */
class IOPValueEffectM: public ActionEffectM
{
    Q_OBJECT

    // Agent IOP
    I2_QML_PROPERTY(AgentIOPM* , agentIOP)

    // value converted into string
    I2_QML_PROPERTY(QString, value)

public:

    /**
     * @brief Default constructor
     * @param parent
     */
    explicit IOPValueEffectM(QObject *parent = 0);


    /**
      * @brief Destructor
      */
    ~IOPValueEffectM();



Q_SIGNALS:


public Q_SLOTS:


protected:



};

QML_DECLARE_TYPE(IOPValueEffectM)

#endif // IOPVALUEEFFECTM_H
