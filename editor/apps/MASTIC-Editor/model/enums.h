/*
 *	MASTIC Editor
 *
 *  Copyright © 2017 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou <peyruqueou@ingenuity.io>
 *
 */

#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>
#include <QtQml>

#include <I2PropertyHelpers.h>


/**
  * Types:
  * - Agent Input
  * - Agent Output
  * - Agent Parameter
  */
I2_ENUM(AgentIOPTypes, INPUT, OUTPUT, PARAMETER)


/**
  * Types of the value of an Agent Input / Output / Parameter
  */
I2_ENUM(AgentIOPValueTypes, INTEGER = 1, DOUBLE, STRING, BOOL, IMPULSION, DATA, MIXED, UNKNOWN)


/**
  * Groups for types of the value of an Agent Input / Output / Parameter
  */
I2_ENUM(AgentIOPValueTypeGroups, NUMBER, STRING, IMPULSION, DATA, MIXED, UNKNOWN)


/**
 * @brief The Enums class is a helper for general enumerations
 */
class Enums : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent
     */
    explicit Enums(QObject *parent = nullptr);

};

#endif // ENUMS_H
