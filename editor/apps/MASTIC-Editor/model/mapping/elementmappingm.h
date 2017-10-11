#ifndef ELEMENTMAPPINGM_H
#define ELEMENTMAPPINGM_H

#include <QObject>
#include <QtQml>

#include <I2PropertyHelpers.h>

class ElementMappingM : public QObject
{  
    Q_OBJECT

    // Input agent name
    I2_QML_PROPERTY_READONLY(QString, inputAgent)

    // Input name to connect with
    I2_QML_PROPERTY_READONLY(QString, input)

    // Output agent name to connect with
    I2_QML_PROPERTY_READONLY(QString, outputAgent)

    // Output name to connect with
    I2_QML_PROPERTY_READONLY(QString, output)

public:

    /**
     * @brief Constructor with input and output agent and IOP
     * @param inputAgent
     * @param input
     * @param outputAgent
     * @param output
     * @param parent
     */
    explicit ElementMappingM(QString inputAgent,
                             QString input,
                             QString outputAgent,
                             QString output,
                             QObject *parent = nullptr);


    /**
     * @brief Destructor
     */
    ~ElementMappingM();

Q_SIGNALS:

public Q_SLOTS:

};

QML_DECLARE_TYPE(ElementMappingM)

#endif // ELEMENTMAPPINGM_H
