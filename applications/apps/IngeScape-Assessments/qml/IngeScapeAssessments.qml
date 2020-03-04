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
 *      Chloé Roumieu      <roumieu@ingenuity.io>
 */

import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Window 2.3
import I2Quick 1.0
import INGESCAPE 1.0

import "experimentation" as Experimentation
import "session" as Session
import "popup" as Popup

Item {
    id: rootItem

    anchors.fill: parent


    //--------------------------------------------------------
    //
    //
    // Properties
    //
    //
    //--------------------------------------------------------


    //--------------------------------------------------------
    //
    //
    // Functions
    //
    //
    //--------------------------------------------------------

    function resetCurrentExperimentation()
    {
        if (IngeScapeAssessmentsC.experimentationC)
        {
            IngeScapeAssessmentsC.experimentationC.currentExperimentation = null;
        }
    }

    function resetCurrentSession()
    {
        if (IngeScapeAssessmentsC.experimentationC && IngeScapeAssessmentsC.experimentationC.sessionC)
        {
            if (IngeScapeAssessmentsC.experimentationC.isRecorderON && IngeScapeAssessmentsC.experimentationC.isRecording)
            {
                IngeScapeAssessmentsC.experimentationC.stopToRecord();
            }

            IngeScapeAssessmentsC.experimentationC.sessionC.currentSession = null;
        }
    }

    function openLicensePopup() {
        licensePopup.open();
    }

    function openDatabasePopup() {
        databasePopup.open();
    }


    //--------------------------------------------------------
    //
    //
    // Behaviors
    //
    //
    //--------------------------------------------------------

    Component.onCompleted: {
        // FIXME Several popup may appear at startup depending on the current platform configuration. Need to prioritize them and maybe show them sequentialy, not on top of each other.

        if (AssessmentsModelC && !AssessmentsModelC.isConnectedToDatabase)
        {
            openDatabasePopup();
        }
        if (IngeScapeAssessmentsC.licensesC && !IngeScapeAssessmentsC.licensesC.isLicenseValidForAgentNeeded) // (Here for IngeScape-Assessments agent)
        {
            openLicensePopup();
        }
        /*
        if (IngeScapeAssessmentsC.gettingStartedShowAtStartup)
        {
            openGettingStarted();
        }*/
    }

    Connections {
        target: IngeScapeAssessmentsC.experimentationC

        onCurrentExperimentationChanged: {
            if (IngeScapeAssessmentsC.experimentationC.currentExperimentation)
            {                
                stackview.push(componentExperimentationView); // Add the "Experimentation View" to the stack
            }
            else {
                console.log("QML: on Current Experimentation changed to NULL");
                stackview.pop(); // Remove the "Experimentation View" from the stack
            }
        }
    }

    Connections {
        target: IngeScapeAssessmentsC.experimentationC.sessionC

        onCurrentSessionChanged: {
            if (IngeScapeAssessmentsC.experimentationC.sessionC.currentSession)
            {
                console.log("QML: on Current Session changed: " + IngeScapeAssessmentsC.experimentationC.sessionC.currentSession.name);
                stackview.push(componentSessionView); // Add the "Session view" to the stack
            }
            else {
                console.log("QML: on Current Session changed to NULL");
                stackview.pop(); // Remove the "Session view" from the stack
            }
        }
    }

    Connections {
        target: IngeScapeAssessmentsC.licensesC

        onIsLicenseValidForAgentNeededChanged: {
            console.log("QML (IngeScape Assessment): on IsLicenseValidForAgentNeededChanged");
            if (IngeScapeAssessmentsC.licensesC && !IngeScapeAssessmentsC.licensesC.isLicenseValidForAgentNeeded)
            {
                openLicensePopup();
            }
        }
    }


    //--------------------------------------------------------
    //
    //
    // Content
    //
    //
    //--------------------------------------------------------

    StackView {
        id: stackview

        anchors.fill: parent

        initialItem: componentExperimentationsListView
    }

    NetworkConnectionInformationItem {
        id: networkConnectionInformationItem
        anchors {
            top: parent.top
            topMargin: 20
            right: parent.right
            rightMargin: 13
        }

        visible : (IngeScapeAssessmentsC.experimentationC
                   // Only on experimentation view in selection mode if NO Recorder
                  && ((IngeScapeAssessmentsC.experimentationC.currentExperimentation && IngeScapeAssessmentsC.experimentationC.isSelectingSessions && !IngeScapeAssessmentsC.experimentationC.isRecorderON)
                        // And always in session view
                        || (IngeScapeAssessmentsC.experimentationC.sessionC && IngeScapeAssessmentsC.experimentationC.sessionC.currentSession)))

        currentNetworkDevice: IngeScapeAssessmentsC.networkDevice
        currentPort: IngeScapeAssessmentsC.port

        onConnectChanged: {
            if (wasOnlineBeforeConnectChanged)
            {
                IngeScapeAssessmentsC.stopIngeScape(false);
            }
            else
            {
                IngeScapeAssessmentsC.startIngeScape();
            }
        }

        onChangeNetworkSettings: {
            IngeScapeAssessmentsC.networkDevice = networkDevice;
            IngeScapeAssessmentsC.port = port;
            IngeScapeAssessmentsC.restartIngeScape(false);
        }
    }

    //
    // Experimentations List View
    //
    Component {
        id: componentExperimentationsListView

        ExperimentationsListView {
            id: experimentationsListView

            controller: IngeScapeAssessmentsC.experimentationsListC
        }
    }

    //
    // Experimentation View
    //
    Component {
        id: componentExperimentationView

        Experimentation.ExperimentationView {
            experimentationC: IngeScapeAssessmentsC.experimentationC

            onGoBackToHome: {
                console.log("QML: on Go Back to 'Home' (from 'Experimentation' view)");
                rootItem.resetCurrentExperimentation();
            }
        }
    }

    //
    // Session View
    //
    Component {
        id: componentSessionView

        Session.SessionView {
            sessionController: IngeScapeAssessmentsC.experimentationC.sessionC
            experimentationController: IngeScapeAssessmentsC.experimentationC

            onGoBackToHome: {
                console.log("QML: on Go Back to 'Home' (from 'Session' view)");
                rootItem.resetCurrentSession();
                rootItem.resetCurrentExperimentation();
            }

            onGoBackToExperimentation: {
                console.log("QML: on Go Back to 'Experimentation' (from 'Session' view)");
                rootItem.resetCurrentSession();
            }
        }
    }

    //
    // License (Configuration) Popup
    //
    LicensePopup {
        id: licensePopup

        anchors.centerIn: parent

        licensesController: IngeScapeAssessmentsC.licensesC
    }

    //
    // Popup displayed when an event occurs about the license(s)
    //
    LicenseEventPopup {
        id: licenseEventPopup

        licensesController: IngeScapeAssessmentsC.licensesC

        onClosed: {
            console.log("Popup displayed when an event occurs about the license(s) has just been closed");
            licensePopup.open();
        }
    }

    //
    // Database (Configuration) Popup
    //
    Popup.DatabaseConfigurationPopup {
        id: databasePopup

        anchors.centerIn: parent
    }
}
