/*
 *	IngeScape Common
 *
 *  Copyright © 2017-2019 Ingenuity i/o. All rights reserved.
 *
 *	See license terms for the rights and conditions
 *	defined by copyright holders.
 *
 *
 *	Contributors:
 *      Vincent Peyruqueou  <peyruqueou@ingenuity.io>
 *      Alexandre Lemort    <lemort@ingenuity.io>
 *
 */

#include "ingescapenetworkcontroller.h"

#include <QDebug>
#include <QApplication>

#include <memory>


#define INGESCAPENETWORKCONTROLLER_IGS_MONITOR_TIMEOUT_IN_MILLISECONDS 500


// Suffix used by IngeScape launcher
static const QString suffix_Launcher = ".ingescapelauncher";

static const int PUBLISHED_VALUE_MAXIMUM_STRING_LENGTH = 4096;


/**
 * @brief Callback for incomming messages on the bus
 * @param evt
 * @param peer
 * @param name
 * @param address
 * @param channel
 * @param headers
 * @param msg
 * @param myData
 */
void onIncommingBusMessageCallback(const char *event, const char *peer, const char *name, const char *address, const char *channel, zhash_t *headers, zmsg_t *msg, void *myData)
{
    Q_UNUSED(channel)

    IngeScapeNetworkController* networkController = static_cast<IngeScapeNetworkController*>(myData);
    if (networkController != nullptr)
    {
        QString peerId = QString(peer);
        QString peerName = QString(name);
        QString peerAddress = QString(address);

        // ENTER
        if (streq(event, "ENTER"))
        {
            qDebug() << QString("--> %1 has entered the network with peer id %2 (and address %3)").arg(peerName, peerId, peerAddress);

            QString ipAddress = "";

            // Get IP address (Example of peerAddress: "tcp://10.0.0.17:49153")
            if (peerAddress.length() > 6)
            {
                // Remove "tcp://" and then split IP address and port
                QStringList ipAddressAndPort = peerAddress.remove(0, 6).split(":");

                if (ipAddressAndPort.count() == 2) {
                    ipAddress = ipAddressAndPort.first();
                }
            }

            // Initialize properties related to message headers
            bool isIngeScapeLauncher = false;
            bool isIngeScapeRecorder = false;
            bool isIngeScapeEditor = false;
            bool isIngeScapeAssessments = false;
            bool isIngeScapeExpe = false;
            QString hostname = "";
            bool canBeFrozen = false;
            QString commandLine = "";
            QString loggerPort = "";
            QString streamingPort = "";

            zlist_t *keys = zhash_keys(headers);
            size_t nbKeys = zlist_size(keys);
            if (nbKeys > 0)
            {
                char *k;
                char *v;
                QString key = "";
                QString value = "";

                while ((k = static_cast<char*>(zlist_pop(keys))))
                {
                    v = static_cast<char*>(zhash_lookup(headers, k));

                    key = QString(k);
                    value = QString(v);

                    if (key == "isLauncher") {
                        if (value == "1") {
                            isIngeScapeLauncher = true;
                        }
                    }
                    else if (key == "isRecorder") {
                        if (value == "1") {
                            isIngeScapeRecorder = true;
                        }
                    }
                    else if (key == "isEditor") {
                        if (value == "1") {
                            isIngeScapeEditor = true;
                        }
                    }
                    else if (key == "isAssessments") {
                        if (value == "1") {
                            isIngeScapeAssessments = true;
                        }
                    }
                    else if (key == "isExpe") {
                        if (value == "1") {
                            isIngeScapeExpe = true;
                        }
                    }
                    else if (key == "hostname") {
                        hostname = value;
                    }
                    else if (key == "canBeFrozen") {
                        if (value == "1") {
                            canBeFrozen = true;
                        }
                    }
                    else if (key == "commandline") {
                        commandLine = value;
                    }
                    else if (key == "logger") {
                        loggerPort = value;
                    }
                    else if (key == "videoStream") {
                        streamingPort = value;
                    }

                    if (k) {
                        free(k);
                    }
                }
            }
            zlist_destroy(&keys);


            // IngeScape LAUNCHER
            if (isIngeScapeLauncher)
            {
                qDebug() << "Our zyre event is about IngeScape LAUNCHER";

                // Save the peer id of this launcher
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::LAUNCHER);
                networkController->setnumberOfLaunchers(networkController->numberOfLaunchers() + 1);

                if (peerName.endsWith(suffix_Launcher)) {
                    hostname = peerName.left(peerName.length() - suffix_Launcher.length());
                }

                // Emit the signal "Launcher Entered"
                Q_EMIT networkController->launcherEntered(peerId, hostname, ipAddress, streamingPort);
            }
            // IngeScape RECORDER
            else if (isIngeScapeRecorder)
            {
                qDebug() << "Our zyre event is about IngeScape RECORDER";

                // Save the peer id of this recorder
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::RECORDER);
                networkController->setnumberOfRecorders(networkController->numberOfRecorders() + 1);

                // Emit the signal "Recorder Entered"
                Q_EMIT networkController->recorderEntered(peerId, peerName, ipAddress, hostname);
            }
            // IngeScape EDITOR
            else if (isIngeScapeEditor)
            {
                qDebug() << "Our zyre event is about IngeScape EDITOR";

                // Save the peer id of this editor
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::EDITOR);
                networkController->setnumberOfEditors(networkController->numberOfEditors() + 1);

                // Emit the signal "Editor Entered"
                Q_EMIT networkController->editorEntered(peerId, peerName, ipAddress, hostname);
            }
            // IngeScape ASSESSMENTS
            else if (isIngeScapeAssessments)
            {
                qDebug() << "Our zyre event is about IngeScape ASSESSMENTS";

                // Save the peer id of this recorder
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::ASSESSMENTS);
                networkController->setnumberOfAssessments(networkController->numberOfAssessments() + 1);

                // Emit the signal "Assessments Entered"
                Q_EMIT networkController->assessmentsEntered(peerId, peerName, ipAddress, hostname);
            }
            // IngeScape EXPE
            else if (isIngeScapeExpe)
            {
                qDebug() << "Our zyre event is about IngeScape EXPE";

                // Save the peer id of this recorder
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::EXPE);
                networkController->setnumberOfExpes(networkController->numberOfExpes() + 1);

                // Emit the signal "Expe Entered"
                Q_EMIT networkController->expeEntered(peerId, peerName, ipAddress, hostname);
            }
            // IngeScape AGENT
            else if (nbKeys > 0)
            {
                qDebug() << "Our zyre event is about IngeScape AGENT on" << hostname;

                // Save the peer id of this agent
                networkController->manageEnteredPeerId(peerId, IngeScapeTypes::AGENT);
                networkController->setnumberOfAgents(networkController->numberOfAgents() + 1);

                // Emit the signal "Agent Entered"
                Q_EMIT networkController->agentEntered(peerId, peerName, ipAddress, hostname, commandLine, canBeFrozen, loggerPort);
            }
            else {
                qDebug() << "Our zyre event is about an element without headers, we ignore it !";
            }
        }
        // JOIN (group)
        else if (streq(event, "JOIN"))
        {
            //qDebug() << QString("++ %1 has joined %2").arg(peerName, group);
        }
        // LEAVE (group)
        else if (streq(event, "LEAVE"))
        {
            //qDebug() << QString("-- %1 has left %2").arg(peerName, group);
        }
        // SHOUT
        else if (streq(event, "SHOUT"))
        {
            zmsg_t* msg_dup = zmsg_dup(msg);

            // Manage the "Shouted" message
            networkController->manageShoutedMessage(peerId, peerName, msg_dup);

            zmsg_destroy(&msg_dup);
        }
        // WHISPER
        else if (streq(event, "WHISPER"))
        {
            zmsg_t* msg_dup = zmsg_dup(msg);

            // Manage the "Whispered" message
            networkController->manageWhisperedMessage(peerId, peerName, msg_dup);

            zmsg_destroy(&msg_dup);
        }
        // EXIT
        else if (streq(event, "EXIT"))
        {
            qDebug() << QString("<-- %1 (%2) exited").arg(peerName, peerId);

            // Get the IngeScape type of a peer id
            IngeScapeTypes::Value ingeScapeType = networkController->getIngeScapeTypeOfPeerId(peerId);

            switch (ingeScapeType)
            {
            // IngeScape AGENT
            case IngeScapeTypes::AGENT:
            {
                // Emit the signal "Agent Exited"
                Q_EMIT networkController->agentExited(peerId, peerName);

                networkController->setnumberOfAgents(networkController->numberOfAgents() - 1);

                break;
            }
            // IngeScape LAUNCHER
            case IngeScapeTypes::LAUNCHER:
            {
                QString hostname = "";

                if (peerName.endsWith(suffix_Launcher)) {
                    hostname = peerName.left(peerName.length() - suffix_Launcher.length());
                }

                // Emit the signal "Launcher Exited"
                Q_EMIT networkController->launcherExited(peerId, hostname);

                networkController->setnumberOfLaunchers(networkController->numberOfLaunchers() - 1);

                break;
            }
            // IngeScape RECORDER
            case IngeScapeTypes::RECORDER:
            {
                // Emit the signal "Recorder Exited"
                Q_EMIT networkController->recorderExited(peerId, peerName);

                networkController->setnumberOfRecorders(networkController->numberOfRecorders() - 1);

                break;
            }
            // IngeScape EDITOR
            case IngeScapeTypes::EDITOR:
            {
                // Emit the signal "Editor Exited"
                Q_EMIT networkController->editorExited(peerId, peerName);

                networkController->setnumberOfEditors(networkController->numberOfEditors() - 1);

                break;
            }
            // IngeScape ASSESSMENTS
            case IngeScapeTypes::ASSESSMENTS:
            {
                // Emit the signal "Assessments Exited"
                Q_EMIT networkController->assessmentsExited(peerId, peerName);

                networkController->setnumberOfAssessments(networkController->numberOfAssessments() - 1);

                break;
            }
            // IngeScape EXPE
            case IngeScapeTypes::EXPE:
            {
                // Emit the signal "Expe Exited"
                Q_EMIT networkController->expeExited(peerId, peerName);

                networkController->setnumberOfExpes(networkController->numberOfExpes() - 1);

                break;
            }
            default:
                qWarning() << "Unknown peer id" << peerId << "(" << peerName << ")";
                break;
            }

            // Manage the peer id which exited the network
            networkController->manageExitedPeerId(peerId);
        }
    }
}




/**
 * @brief Callback for igs_monitor
 * @param event
 * @param device
 * @param ipAddress
 * @param myData
 */
void onMonitorCallback(igs_monitorEvent_t event, const char *device, const char *ipAddress, void *myData)
{
    IngeScapeNetworkController* networkController = static_cast<IngeScapeNetworkController*>(myData);
    if (networkController != nullptr)
    {
#ifdef Q_OS_WIN
        QString networkDevice = QString::fromLatin1(device);
#else
        QString networkDevice = QString(device);
#endif

        switch (event)
        {
            case IGS_NETWORK_OK:
                {
                    qInfo() << "Device available again: " << networkDevice
                            << " - ingescape agent can be started again";

                    // Update our list of network devices
                    networkController->updateAvailableNetworkDevices();

                    // Notify event
                    networkController->networkDeviceIsAvailableAgain();

                    // Automatically start our agent if needed
                    if (networkController->automaticallyStartStopOnMonitorEvents())
                    {
                        networkController->startWithPreviousConfiguration();
                    }
                }
                break;

            case IGS_NETWORK_DEVICE_NOT_AVAILABLE:
                {
                    qInfo() << "Device not available: " << networkDevice
                            << " - ingescape agent must be stopped";

                    // Update internal state
                    networkController->setisOnline(false);

                    // Update our list of network devices
                    networkController->updateAvailableNetworkDevices();

                    // Notify event
                    networkController->networkDeviceIsNotAvailable();

                    // Automatically stop our agent if needed
                    if (networkController->automaticallyStartStopOnMonitorEvents())
                    {
                        networkController->stop();
                    }
                }
                break;

            case IGS_NETWORK_ADDRESS_CHANGED:
                {
                    qInfo() << "IngeScapeNetworkController: device " << networkDevice
                            << " has a new ip address " << ipAddress << " - ingescape agent must be restarted";

                    // Notify event
                    networkController->networkDeviceIpAddressHasChanged(QString(ipAddress));

                    // Automatically restart our agent if needed
                    if (networkController->automaticallyStartStopOnMonitorEvents())
                    {
                        networkController->restart();
                    }
                }
                break;

            case IGS_NETWORK_OK_AFTER_MANUAL_RESTART:
                {
                    qInfo() << "IngeScape manually reconnected on device: " << networkDevice
                            << " - nothing to do";
                }
                break;

            default:
                break;
        }
    }
}


/**
 * @brief Callback for Observing Inputs of our agent "IngeScape Editor"
 * @param iopType
 * @param name
 * @param valueType
 * @param value
 * @param valueSize
 * @param myData
 */
void onObserveInputCallback(iop_t iopType, const char* name, iopType_t valueType, void* value, size_t valueSize, void* myData)
{
    Q_UNUSED(value)
    Q_UNUSED(valueSize)

    IngeScapeNetworkController* networkController = static_cast<IngeScapeNetworkController*>(myData);
    if (networkController != nullptr)
    {
        if (iopType == IGS_INPUT_T)
        {
            QString inputName(name);

            QStringList agentNameAndIOP = inputName.split(SEPARATOR_AGENT_NAME_AND_IOP);
            if (agentNameAndIOP.count() == 2)
            {
                QString outputAgentName = agentNameAndIOP.at(0);
                QString outputId = agentNameAndIOP.at(1);

                AgentIOPValueTypes::Value agentIOPValueType = static_cast<AgentIOPValueTypes::Value>(valueType);
                QVariant currentValue = QVariant();
                bool isValid = false;

                switch (valueType)
                {
                // INTEGER
                case IGS_INTEGER_T: {
                    int newValue = *(static_cast<int *>(value));

                    currentValue = QVariant(newValue);
                    isValid = true;

                    //qDebug() << "New value" << newValue << "received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    break;
                }
                // DOUBLE
                case IGS_DOUBLE_T: {
                    double newValue = *(static_cast<double*>(value));

                    currentValue = QVariant(newValue);
                    isValid = true;

                    //qDebug() << "New value" << newValue << "received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    break;
                }
                // STRING
                case IGS_STRING_T: {
                    char *rawNewValue = static_cast<char *>(value);
                    QString newValue = QString::fromUtf8(rawNewValue, qMin(static_cast<int>(strlen(rawNewValue)), PUBLISHED_VALUE_MAXIMUM_STRING_LENGTH));

                    currentValue = QVariant(newValue);
                    isValid = true;

                    //qDebug() << "New value" << newValue << "received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    break;
                }
                // BOOL
                case IGS_BOOL_T: {
                    bool newValue = *(static_cast<bool*>(value));

                    currentValue = QVariant(newValue);
                    isValid = true;

                    //qDebug() << "New value" << newValue << "received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    break;
                }
                // IMPULSION
                case IGS_IMPULSION_T: {
                    isValid = true;

                    //qDebug() << "New IMPULSION received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    break;
                }
                // DATA
                case IGS_DATA_T: {
                    // On peut utiliser directement value plutôt que de re-générer un tableau de bytes ??
                    // On stocke dans un dossier le media (eg video, son, image) et on log le path et le start time ??
                    //void* data = NULL;
                    //int result = igs_readInputAsData(name, &data, &valueSize);
                    //if (result == 1) {
                        // data must be a char* to have automatic conversion
                        //QByteArray newValue = QByteArray(data, valueSize);
                        //currentValue = QVariant(newValue);
                        //isValid = true;

                        //qDebug() << "New DATA with size" << valueSize << "received on" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                    //}
                    //else {
                    //    qCritical() << "Can NOT read input" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType) << "(DATA of size:" << valueSize << ")";
                    //}

                    isValid = true;

                    break;
                }
                default: {
                    break;
                }
                }

                if (isValid)
                {
                    PublishedValueM* publishedValue = new PublishedValueM(QDateTime::currentDateTime(),
                                                                          outputAgentName,
                                                                          outputId,
                                                                          agentIOPValueType,
                                                                          currentValue);

                    // Emit the signal "Value Published"
                    Q_EMIT networkController->valuePublished(publishedValue);
                }
                else {
                    qCritical() << "Can NOT read input" << inputName << "with type" << AgentIOPValueTypes::staticEnumToString(agentIOPValueType);
                }
            }
        }
    }
}


//--------------------------------------------------------------
//
// IngeScape Network Controller
//
//--------------------------------------------------------------

/**
 * @brief Constructor
 * @param igsServiceDescription
 * @param parent
 */
IngeScapeNetworkController::IngeScapeNetworkController(QObject *parent) : QObject(parent),
    _isStarted(false),
    _isOnline(false),
    _automaticallyStartStopOnMonitorEvents(false),
    _agentModel(nullptr),
    _numberOfAgents(0),
    _numberOfLaunchers(0),
    _numberOfRecorders(0),
    _numberOfEditors(0),
    _numberOfAssessments(0),
    _numberOfExpes(0),
    _igsAgentApplicationName(""),
    _lastArgumentsOfStart_networkDevice(""),
    _lastArgumentsOfStart_ipAddress(""),
    _lastArgumentsOfStart_port(0)
{
    // Force ownership of our object, it will prevent Qml from stealing it
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    // Init the name of the "IngeScape" agent that correspond to our application with the application name
    _igsAgentApplicationName = QApplication::instance()->applicationName();
    QString organizationName = QApplication::instance()->organizationName();
    QString version = QApplication::instance()->applicationVersion();


    //
    // Create our internal definition
    //
    QString definitionByDefault = "{  \
                                  \"definition\": {  \
                                  \"name\": \""+ _igsAgentApplicationName + "\",   \
                                  \"description\": \"Definition of " + _igsAgentApplicationName + " made by "+ organizationName +"\",  \
                                  \"version\": \"" + version + "\",  \
                                  \"parameters\": [],   \
                                  \"inputs\": [],       \
                                  \"outputs\": [] }}";

    igs_loadDefinition(definitionByDefault.toStdString().c_str());


    //
    // Create our internal mapping
    //
    QString mappingByDefault = "{      \
                                  \"mapping\": {    \
                                  \"name\": \"" + _igsAgentApplicationName + "\",   \
                                  \"description\": \"Mapping of " + _igsAgentApplicationName + " made by "+ organizationName + "\",  \
                                  \"version\": \"" + version + "\",  \
                                  \"mapping_out\": [],   \
                                  \"mapping_cat\": [] }}";

    igs_loadMapping(mappingByDefault.toStdString().c_str());


    // Configure IGS monitoring
    igs_monitoringShallStartStopAgent(false);
    igs_monitor(&onMonitorCallback, this);


    // Begin to observe incoming messages on the bus
    int result = igs_observeBus(&onIncommingBusMessageCallback, this);
    if (result == 0) {
        qCritical() << "The callback on zyre messages has NOT been registered !";
    }

    // Create the model of our agent "IngeScape"
    _agentModel = new AgentM(_igsAgentApplicationName);

    QString definitionDescription = QString("Definition of %1 made by %2").arg(_igsAgentApplicationName, organizationName);
    DefinitionM* agentDefinition = new DefinitionM(_igsAgentApplicationName,
                                                   version,
                                                   definitionDescription);

    _agentModel->setdefinition(agentDefinition);
}


/**
 * @brief Destructor
 */
IngeScapeNetworkController::~IngeScapeNetworkController()
{
    // Stop monitoring
    stopMonitoring();

    // Stop our IngeScape agent
    stop();

    // Delete the model of our agent "IngeScape Editor"
    if (_agentModel != nullptr)
    {
        AgentM* temp = _agentModel;
        setagentModel(nullptr);
        delete temp;
    }
}


/**
 * @brief Start our IngeScape agent with a network device (or an IP address) and a port
 * @param networkDevice
 * @param ipAddress
 * @param port
 * @return
 */
bool IngeScapeNetworkController::start(QString networkDevice, QString ipAddress, uint port)
{
    if (!_isStarted)
    {
        int agentStarted = 0;

        // Start service with network device
        if (!networkDevice.isEmpty())
        {
#ifdef Q_OS_WIN
            // igs_startWithDevice compares networkDevice with latin1 values to get the IP address of the network device
            agentStarted = igs_startWithDevice(networkDevice.toLatin1().toStdString().c_str(), port);

#else
            agentStarted = igs_startWithDevice(networkDevice.toStdString().c_str(), port);
#endif
        }

        // Start service with ip address (if start with network device has failed)
        if ((agentStarted != 1) && !ipAddress.isEmpty())
        {
            agentStarted = igs_startWithIP(ipAddress.toStdString().c_str(), port);
        }

        // Log status
        if (agentStarted == 1)
        {
            // Update internal state
            setisStarted(true);
            setisOnline(true);

            // Log
            qInfo() << "IngeScape Agent" << _igsAgentApplicationName << "started";
        }
        else
        {
            // Update internal state
            setisOnline(false);

            // Log
            qCritical() << "The network has NOT been initialized on" << networkDevice << "or" << ipAddress << "and port" << QString::number(port);
        }

        // Save values
        _lastArgumentsOfStart_networkDevice = networkDevice;
        _lastArgumentsOfStart_ipAddress = ipAddress;
        _lastArgumentsOfStart_port = port;

        // Start monitoring
        startMonitoring(networkDevice, port);
    }

    return _isStarted;
}


/**
 * @brief Stop our IngeScape agent
 */
void IngeScapeNetworkController::stop()
{
    if (_isStarted)
    {
        qInfo() << "IngeScape Agent" << _igsAgentApplicationName << "will stop";

        // Stop network services
        igs_stop();

        // Update internal states
        setisStarted(false);
        setisOnline(false);

        // Reset counters
        setnumberOfAgents(0);
        setnumberOfLaunchers(0);
        setnumberOfRecorders(0);
        setnumberOfEditors(0);
        setnumberOfAssessments(0);
        setnumberOfExpes(0);
    }
}


/**
 * @brief Restart our Ingescape agent
 */
bool IngeScapeNetworkController::restart()
{
    stop();
    return startWithPreviousConfiguration();
}


/**
 * @brief start with previous configuration
 * @return
 */
bool IngeScapeNetworkController::startWithPreviousConfiguration()
{
   return start(_lastArgumentsOfStart_networkDevice, _lastArgumentsOfStart_ipAddress, _lastArgumentsOfStart_port);
}


/**
 * @brief Start monitoring
 *
 * @param expectedNetworkDevice
 * @param expectedPort
 */
void IngeScapeNetworkController::startMonitoring(QString expectedNetworkDevice, uint expectedPort)
{
    if (!igs_isMonitoringEnabled())
    {
        if (!expectedNetworkDevice.isEmpty())
        {
            #ifdef Q_OS_WIN
                // igs_startWithDevice compares networkDevice with latin1 values to get the IP address of the network device
                igs_monitoringEnableWithExpectedDevice(INGESCAPENETWORKCONTROLLER_IGS_MONITOR_TIMEOUT_IN_MILLISECONDS, expectedNetworkDevice.toLatin1().toStdString().c_str(), expectedPort);

            #else
                igs_monitoringEnableWithExpectedDevice(INGESCAPENETWORKCONTROLLER_IGS_MONITOR_TIMEOUT_IN_MILLISECONDS, expectedNetworkDevice.toStdString().c_str(), expectedPort);
            #endif
        }
        else
        {
            igs_monitoringEnable(INGESCAPENETWORKCONTROLLER_IGS_MONITOR_TIMEOUT_IN_MILLISECONDS);
        }
    }
    // Else: monitoring is already started
}


/**
 * @brief Stop monitoring
 */
void IngeScapeNetworkController::stopMonitoring()
{
    if (igs_isMonitoringEnabled())
    {
        igs_monitoringDisable();
    }
    // Else: monitoring is not started
}


/**
 * @brief Get the IngeScape type of a peer id
 * @param peerId
 * @return
 */
IngeScapeTypes::Value IngeScapeNetworkController::getIngeScapeTypeOfPeerId(QString peerId)
{
    if (_hashFromPeerIdToIngeScapeType.contains(peerId)) {
        return _hashFromPeerIdToIngeScapeType.value(peerId);
    }
    else {
        return IngeScapeTypes::UNKNOWN;
    }
}


/**
 * @brief Manage a peer id which entered the network
 * @param peerId
 * @param ingeScapeType
 */
void IngeScapeNetworkController::manageEnteredPeerId(QString peerId, IngeScapeTypes::Value ingeScapeType)
{
    if (!_hashFromPeerIdToIngeScapeType.contains(peerId)) {
        _hashFromPeerIdToIngeScapeType.insert(peerId, ingeScapeType);
    }
}


/**
 * @brief Manage a peer id which exited the network
 * @param peerId
 */
void IngeScapeNetworkController::manageExitedPeerId(QString peerId)
{
    if (_hashFromPeerIdToIngeScapeType.contains(peerId)) {
        _hashFromPeerIdToIngeScapeType.remove(peerId);
    }
}


/**
 * @brief Update the list of available network devices
 */
void IngeScapeNetworkController::updateAvailableNetworkDevices()
{
    QStringList networkDevices;

    char **devices = nullptr;
    int nb = 0;
    igs_getNetdevicesList(&devices, &nb);

    for (int i = 0; i < nb; i++)
    {
#ifdef Q_OS_WIN
        // igs_getNetdevicesList return latin1 values
        QString availableNetworkDevice = QString::fromLatin1(devices[i]);
#else
        QString availableNetworkDevice = QString(devices[i]);
#endif

        networkDevices.append(availableNetworkDevice);
    }
    igs_freeNetdevicesList(devices, nb);

    setavailableNetworkDevices(networkDevices);

    qInfo() << "Update available Network Devices:" << _availableNetworkDevices;
}


/**
 * @brief Return true if the network device is available
 * @param networkDevice
 * @return
 */
bool IngeScapeNetworkController::isAvailableNetworkDevice(QString networkDevice)
{
    return (!networkDevice.isEmpty() && _availableNetworkDevices.contains(networkDevice));
}


/**
 * @brief Manage a "Shouted" message
 * @param peerId
 * @param peerName
 * @param zMessage
 */
void IngeScapeNetworkController::manageShoutedMessage(QString peerId, QString peerName, zmsg_t* zMessage)
{
    std::unique_ptr<char> zmsg_str(zmsg_popstr(zMessage));
    QString message(zmsg_str.get());

    qDebug() << "Not yet managed SHOUTED message '" << message << "' for agent" << peerName << "(" << peerId << ")";
}


/**
 * @brief Manage a "Whispered" message
 * @param peerId
 * @param peerName
 * @param zMessage
 */
void IngeScapeNetworkController::manageWhisperedMessage(QString peerId, QString peerName, zmsg_t* zMessage)
{
    std::unique_ptr<char> zmsg_str(zmsg_popstr(zMessage));
    QString message(zmsg_str.get());

    // An agent DEFINITION has been received
    /*if (message.startsWith(prefix_Definition))
    {
        QString definitionJSON = message.remove(0, prefix_Definition.length());

        Q_EMIT definitionReceived(peerId, peerName, definitionJSON);
    }
    // An agent MAPPING has been received
    else if (message.startsWith(prefix_Mapping))
    {
        QString mappingJSON = message.remove(0, prefix_Mapping.length());

        Q_EMIT mappingReceived(peerId, peerName, mappingJSON);
    }*/

    qDebug() << "Not yet managed WHISPERED message '" << message << "' for agent" << peerName << "(" << peerId << ")";
}


/**
 * @brief Send a (string) message to an agent (identified by its peer id)
 * @param agentId peer id of the agent
 * @param message 1 string
 * @return true if successful, false otherwise
 */
bool IngeScapeNetworkController::sendMessageToAgent(QString agentId, QString message)
{
    bool success = false;

    if (!agentId.isEmpty() && !message.isEmpty())
    {
        // Use IngeScape to send the message to the agent
        int result = igs_busSendStringToAgent(agentId.toStdString().c_str(),
                                              "%s",
                                              message.toStdString().c_str());

        if (result == 1)
        {
            success = true;

            qInfo() << "Message" << message << "to peer" << agentId << "sent successfully";
        }
        else {
            qWarning() << "Error (" << result << ") during send message" << message << "to peer" << agentId;
        }
    }
    return success;
}


/**
 * @brief Send a (strings list) message to an agent (identified by its peer id)
 * @param agentId peer id of the agent
 * @param message list of strings
 * @return true if successful, false otherwise
 */
bool IngeScapeNetworkController::sendMessageToAgent(QString agentId, QStringList message)
{
    bool success = false;

    if (!agentId.isEmpty() && !message.isEmpty())
    {
        int stringsNumber = message.count();
        QString string1 = message.at(0);

        if (stringsNumber > 1)
        {
            QString spaceAndString = " %s";
            QString format = QString("%s%1").arg(spaceAndString.repeated(stringsNumber - 1));

            switch (stringsNumber)
            {
            case 2:
            {
                QString string2 = message.at(1);

                // Use IngeScape to send the message to the agent
                int result = igs_busSendStringToAgent(agentId.toStdString().c_str(),
                                                      format.toStdString().c_str(),
                                                      string1.toStdString().c_str(),
                                                      string2.toStdString().c_str());

                if (result == 1)
                {
                    success = true;

                    qInfo() << "Message" << message << "to peer" << agentId << "sent successfully";
                }
                else {
                    qWarning() << "Error (" << result << ") during send message" << message << "to peer" << agentId;
                }
            }
                break;

            case 3:
            {
                QString string2 = message.at(1);
                QString string3 = message.at(2);

                // Use IngeScape to send the message to the agent
                int result = igs_busSendStringToAgent(agentId.toStdString().c_str(),
                                                      format.toStdString().c_str(),
                                                      string1.toStdString().c_str(),
                                                      string2.toStdString().c_str(),
                                                      string3.toStdString().c_str());

                if (result == 1)
                {
                    success = true;

                    qInfo() << "Message" << message << "to peer" << agentId << "sent successfully";
                }
                else {
                    qWarning() << "Error (" << result << ") during send message" << message << "to peer" << agentId;
                }
            }
                break;

            /*case 4:
            {
                QString string2 = message.at(1);
                QString string3 = message.at(2);
                QString string4 = message.at(3);

                // Use IngeScape to send the message to the agent
                int result = igs_busSendStringToAgent(agentId.toStdString().c_str(),
                                                   format.toStdString().c_str(),
                                                   string1.toStdString().c_str(),
                                                   string2.toStdString().c_str(),
                                                   string3.toStdString().c_str(),
                                                   string4.toStdString().c_str());

                if (result == 1)
                {
                    success = true;

                    qInfo() << "Message" << message << "to peer" << agentId << "sent successfully";
                }
                else {
                    qWarning() << "Error (" << result << ") during send message" << message << "to peer" << agentId;
                }
            }
                break;*/

            default:
                qCritical() << "Message" << message << "could not be sent due to too many parameters:" << stringsNumber;
                break;
            }
        }
        else
        {
            success = sendMessageToAgent(agentId, string1);
        }
    }
    return success;
}


/**
 * @brief Send a (string) message to a list of agents (identified by their peer id)
 * @param agentIds peer ids of the agents
 * @param message 1 string
 * @return true if successful, false otherwise
 */
bool IngeScapeNetworkController::sendMessageToAgents(QStringList agentIds, QString message)
{
    bool allSucceeded = true;

    for (QString agentId : agentIds)
    {
        bool success = sendMessageToAgent(agentId, message);
        if (!success) {
            allSucceeded = false;
        }
    }
    return allSucceeded;
}


/**
 * @brief Send a (strings list) message to an agent (identified by its peer id)
 * @param agentIds peer ids of the agents
 * @param message list of strings
 * @return true if successful, false otherwise
 */
bool IngeScapeNetworkController::sendMessageToAgents(QStringList agentIds, QStringList message)
{
    bool allSucceeded = true;

    for (QString agentId : agentIds)
    {
        bool success = sendMessageToAgent(agentId, message);
        if (!success) {
            allSucceeded = false;
        }
    }
    return allSucceeded;
}


/**
 * @brief Slot called when a command must be sent on the network to a recorder
 * @param peerIdOfRecorder
 * @param commandAndParameters
 */
void IngeScapeNetworkController::onCommandAskedToRecorder(QString peerIdOfRecorder, QString commandAndParameters)
{
    if (!peerIdOfRecorder.isEmpty() && !commandAndParameters.isEmpty())
    {
        // Send the command (and parameters) to the peer id of the recorder
        int success = igs_busSendStringToAgent(peerIdOfRecorder.toStdString().c_str(), "%s", commandAndParameters.toStdString().c_str());

        qInfo() << "Send command (and parameters)" << commandAndParameters << "to recorder" << peerIdOfRecorder << "with success ?" << success;
    }
}


/**
 * @brief Slot called when the flag "is Mapping Activated" changed
 * @param isMappingConnected
 */
void IngeScapeNetworkController::onIsMappingConnectedChanged(bool isMappingConnected)
{
    if ((_agentModel != nullptr) && (_agentModel->definition() != nullptr))
    {
        for (AgentIOPM* input : _agentModel->definition()->inputsList()->toList())
        {
            if (input != nullptr)
            {
                QString inputName = input->name();

                QStringList agentNameAndIOP = inputName.split(SEPARATOR_AGENT_NAME_AND_IOP);
                if (agentNameAndIOP.count() == 2)
                {
                    QString outputAgentName = agentNameAndIOP.at(0);
                    QString outputId = agentNameAndIOP.at(1);

                    // Get the name and the value type of the output from its id
                    QPair<QString, AgentIOPValueTypes::Value> pair = AgentIOPM::getNameAndValueTypeFromId(outputId);

                    if (!pair.first.isEmpty() && (pair.second != AgentIOPValueTypes::UNKNOWN))
                    {
                        QString outputName = pair.first;
                        //AgentIOPValueTypes::Value valueType = pair.second;

                        // Mapping Activated (Connected)
                        if (isMappingConnected)
                        {
                            // Add mapping between our input and this output
                            unsigned long id = igs_addMappingEntry(inputName.toStdString().c_str(), outputAgentName.toStdString().c_str(), outputName.toStdString().c_str());

                            if (id > 0) {
                                //qDebug() << "Mapping added between output" << outputName << "of agent" << outputAgentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "(id" << id << ")";
                            }
                            else {
                                qCritical() << "Can NOT add mapping between output" << outputName << "of agent" << outputAgentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "Error code:" << id;
                            }
                        }
                        // Mapping DE-activated (DIS-connected)
                        else
                        {
                            // Remove mapping between our input and this output
                            int resultRemoveMappingEntry = igs_removeMappingEntryWithName(inputName.toStdString().c_str(), outputAgentName.toStdString().c_str(), outputName.toStdString().c_str());

                            if (resultRemoveMappingEntry == 1)
                            {
                                //qDebug() << "Mapping removed between output" << outputName << "of agent" << outputAgentName << "and input" << inputName << "of agent" << _igsAgentApplicationName;
                            }
                            else {
                                qCritical() << "Can NOT remove mapping between output" << outputName << "of agent" << outputAgentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "Error code:" << resultRemoveMappingEntry;
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 * @brief Slot called when inputs must be added to our application for a list of agent outputs
 * @param agentName
 * @param newOutputsIds
 * @param isMappingConnected
 */
void IngeScapeNetworkController::onAddInputsToOurApplicationForAgentOutputs(QString agentName, QStringList newOutputsIds, bool isMappingConnected)
{
    if ((_agentModel != nullptr) && (_agentModel->definition() != nullptr) && !newOutputsIds.isEmpty())
    {
        for (QString outputId : newOutputsIds)
        {
            // Get the name and the value type of the output from its id
            QPair<QString, AgentIOPValueTypes::Value> pair = AgentIOPM::getNameAndValueTypeFromId(outputId);

            if (!pair.first.isEmpty() && (pair.second != AgentIOPValueTypes::UNKNOWN))
            {
                QString outputName = pair.first;
                AgentIOPValueTypes::Value valueType = pair.second;

                QString inputName = QString("%1%2%3").arg(agentName, SEPARATOR_AGENT_NAME_AND_IOP, outputId);

                int resultCreateInput = 0;

                switch (valueType)
                {
                case AgentIOPValueTypes::INTEGER: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_INTEGER_T, nullptr, 0);
                    break;
                }
                case AgentIOPValueTypes::DOUBLE: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_DOUBLE_T, nullptr, 0);
                    break;
                }
                case AgentIOPValueTypes::STRING: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_STRING_T, nullptr, 0);
                    break;
                }
                case AgentIOPValueTypes::BOOL: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_BOOL_T, nullptr, 0);
                    break;
                }
                case AgentIOPValueTypes::IMPULSION: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_IMPULSION_T, nullptr, 0);
                    break;
                }
                case AgentIOPValueTypes::DATA: {
                    resultCreateInput = igs_createInput(inputName.toStdString().c_str(), IGS_DATA_T, nullptr, 0);
                    break;
                }
                default: {
                    qCritical() << "Wrong type for the value of output" << outputName << "of agent" << agentName;
                    break;
                }
                }

                if (resultCreateInput == 1)
                {
                    qDebug() << "Input" << inputName << "created on agent" << _igsAgentApplicationName << "with value type" << AgentIOPValueTypes::staticEnumToString(valueType);

                    // Create a new model of input
                    AgentIOPM* input = new AgentIOPM(AgentIOPTypes::INPUT, inputName, valueType);

                    // Add the input to the definition of our agent "IngeScape Editor"
                    _agentModel->definition()->inputsList()->append(input);

                    // Begin the observe of this input
                    int resultObserveInput = igs_observeInput(inputName.toStdString().c_str(), onObserveInputCallback, this);

                    if (resultObserveInput == 1) {
                        //qDebug() << "Observe input" << inputName << "on agent" << _igsAgentApplicationName;
                    }
                    else {
                        qCritical() << "Can NOT observe input" << inputName << "on agent" << _igsAgentApplicationName << "Error code:" << resultObserveInput;
                    }

                    // The mapping is activated (connected)
                    if (isMappingConnected)
                    {
                        // Add mapping between our input and this output
                        unsigned long id = igs_addMappingEntry(inputName.toStdString().c_str(), agentName.toStdString().c_str(), outputName.toStdString().c_str());

                        if (id > 0) {
                            //qDebug() << "Mapping added between output" << outputName << "of agent" << agentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "(id" << id << ")";
                        }
                        else {
                            qCritical() << "Can NOT add mapping between output" << outputName << "of agent" << agentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "Error code:" << id;
                        }
                    }
                }
                else {
                    qCritical() << "Can NOT create input" << inputName << "on agent" << _igsAgentApplicationName << "with value type" << AgentIOPValueTypes::staticEnumToString(valueType) << "Error code:" << resultCreateInput;
                }
            }
        }
    }
}


/**
 * @brief Slot called when inputs must be removed from our application for a list of agent outputs
 * @param agentName
 * @param oldOutputsIds
 * @param isMappingConnected
 */
void IngeScapeNetworkController::onRemoveInputsFromOurApplicationForAgentOutputs(QString agentName, QStringList oldOutputsIds, bool isMappingConnected)
{
    Q_UNUSED(isMappingConnected)

    if ((_agentModel != nullptr) && (_agentModel->definition() != nullptr) && !oldOutputsIds.isEmpty())
    {
        for (QString outputId : oldOutputsIds)
        {
            // Get the name and the value type of the output from its id
            QPair<QString, AgentIOPValueTypes::Value> pair = AgentIOPM::getNameAndValueTypeFromId(outputId);

            if (!pair.first.isEmpty() && (pair.second != AgentIOPValueTypes::UNKNOWN))
            {
                QString outputName = pair.first;
                AgentIOPValueTypes::Value valueType = pair.second;

                QString inputName = QString("%1%2%3").arg(agentName, SEPARATOR_AGENT_NAME_AND_IOP, outputId);

                // The mapping is activated (connected)
                //if (isMappingConnected)
                //{
                // Remove mapping between our input and this output
                int resultRemoveMappingEntry = igs_removeMappingEntryWithName(inputName.toStdString().c_str(), agentName.toStdString().c_str(), outputName.toStdString().c_str());

                if (resultRemoveMappingEntry == 1)
                {
                    //qDebug() << "Mapping removed between output" << outputName << "of agent" << agentName << "and input" << inputName << "of agent" << _igsAgentApplicationName;
                }
                else {
                    qCritical() << "Can NOT remove mapping between output" << outputName << "of agent" << agentName << "and input" << inputName << "of agent" << _igsAgentApplicationName << "Error code:" << resultRemoveMappingEntry;
                }
                //}

                // Remove our input
                int resultRemoveInput = igs_removeInput(inputName.toStdString().c_str());

                if (resultRemoveInput == 1)
                {
                    qDebug() << "Input" << inputName << "removed on agent" << _igsAgentApplicationName << "with value type" << AgentIOPValueTypes::staticEnumToString(valueType);

                    // Get the Input with its name
                    AgentIOPM* input = _agentModel->definition()->getInputWithName(inputName);
                    if (input != nullptr)
                    {
                        // Remove the input from the definition of our agent "IngeScape Editor"
                        _agentModel->definition()->inputsList()->remove(input);
                    }
                }
                else {
                    qCritical() << "Can NOT remove input" << inputName << "on agent" << _igsAgentApplicationName << "with value type" << AgentIOPValueTypes::staticEnumToString(valueType) << "Error code:" << resultRemoveInput;
                }
            }
        }
    }
}
