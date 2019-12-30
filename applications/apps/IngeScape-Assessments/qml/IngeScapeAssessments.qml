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

import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import I2Quick 1.0
import QtQuick.Window 2.3

import INGESCAPE 1.0

import "experimentation" as Experimentation
import "task_instance" as TaskInstance
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

    /**
     * Reset the current experimentation
     */
    function resetCurrentExperimentation()
    {
        if (IngeScapeAssessmentsC.experimentationC)
        {
            IngeScapeAssessmentsC.experimentationC.currentExperimentation = null;
        }
    }

    /**
     * Reset the current session
     */
    function resetCurrentSession()
    {
        if (IngeScapeAssessmentsC.experimentationC && IngeScapeAssessmentsC.experimentationC.taskInstanceC)
        {
            IngeScapeAssessmentsC.experimentationC.taskInstanceC.currentSession = null;
        }
    }


    //
    // Function allowing to open the license (configuration) popup
    //
    function openLicensePopup() {
        licensePopup.open();
    }


    //
    // Function allowing to open the database (configuration) popup
    //
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

    // When the QML is loaded...
    Component.onCompleted: {
        // FIXME Several popup may appear at startup depending on the current platform configuration. Need to prioritize them and maybe show them sequentialy, not on top of each other.

        // ...we check if we are connected to a database
        if (AssessmentsModelC && !AssessmentsModelC.isConnectedToDatabase)
        {
            openDatabasePopup();
        }

        // ...we check the value of the flag "is License For Agent Needed" (Here for IngeScape-Assessments agent)
        if (IngeScapeAssessmentsC.licensesC && !IngeScapeAssessmentsC.licensesC.isLicenseValidForAgentNeeded)
        {
            openLicensePopup();
        }

        /*// ...we check if we must open the getting started window
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
                // Add the "Experimentation View" to the stack
                stackview.push(componentExperimentationView);
            }
            else {
                console.log("QML: on Current Experimentation changed to NULL");

                // Remove the "Experimentation View" from the stack
                stackview.pop();
            }
        }
    }

    Connections {
        target: IngeScapeAssessmentsC.experimentationC.taskInstanceC

        onCurrentSessionChanged: {

            if (IngeScapeAssessmentsC.experimentationC.taskInstanceC.currentSession)
            {
                console.log("QML: on Current Session changed: " + IngeScapeAssessmentsC.experimentationC.taskInstanceC.currentSession.name);

                // Add the "Session view" to the stack
                stackview.push(componentTaskInstanceView);
            }
            else {
                console.log("QML: on Current Session changed to NULL");

                // Remove the "Session view" from the stack
                stackview.pop();
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
        id: networkConfigurationInfo

        anchors {
            top: parent.top
            topMargin: 20
            right: parent.right
            rightMargin: 13
        }

        editorStartedOnIgs: IgsNetworkController ? IgsNetworkController.isStarted : false

        currentNetworkDevice: IngeScapeAssessmentsC.networkDevice
        currentPort: IngeScapeAssessmentsC.port

        listOfNetworkDevices: IgsNetworkController ? IgsNetworkController.availableNetworkDevices : null

        onWillOpenEditionMode: {
            // Update our list of available network devices
            IgsNetworkController.updateAvailableNetworkDevices();
        }

        onChangeNetworkSettings: {
            if (IgsNetworkController.isAvailableNetworkDevice(networkDevice))
            {
                // Re-Start the Network
                var success = IngeScapeAssessmentsC.restartNetwork(port, networkDevice, clearPlatform);
                if (success)
                {
                    close();
                }
                else
                {
                    console.error("Network cannot be (re)started on device " + networkDevice + " and port " + port);
                }
            }
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

            //
            // Behavior
            //

            Component.onCompleted: {
                networkConfigurationInfo.visible = false;
            }
        }
    }


    //
    // Experimentation View
    //
    Component {
        id: componentExperimentationView

        Experimentation.ExperimentationView {
            //id: experimentationView

            controller: IngeScapeAssessmentsC.experimentationC

            //
            // Behavior
            //

            Component.onCompleted: {
                networkConfigurationInfo.visible = true;
            }


            //
            // Slots
            //

            onGoBackToHome: {
                console.log("QML: on Go Back to 'Home' (from 'Experimentation' view)");

                // Hide network configuration infos component
                networkConfigurationInfo.visible = false;

                // Reset the current experimentation
                rootItem.resetCurrentExperimentation();
            }
        }
    }


    //
    // Task Instance View
    //
    Component {
        id: componentTaskInstanceView

        TaskInstance.TaskInstanceView {
            taskInstanceController: IngeScapeAssessmentsC.experimentationC.taskInstanceC
            experimentationController: IngeScapeAssessmentsC.experimentationC

            //
            // Behavior
            //

            Component.onCompleted: {
                networkConfigurationInfo.visible = true;
            }

            //
            // Slots
            //

            onGoBackToHome: {
                console.log("QML: on Go Back to 'Home' (from 'TaskInstance' view)");

                // Hide network configuration infos component
                networkConfigurationInfo.visible = false;

                // Reset the current session
                rootItem.resetCurrentSession();

                // Reset the current experimentation
                rootItem.resetCurrentExperimentation();
            }

            onGoBackToExperimentation: {
                console.log("QML: on Go Back to 'Experimentation' (from 'TaskInstance' view)");

                // Hide network configuration infos component
                networkConfigurationInfo.visible = true;

                // Reset the current session
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

            licensePopup.allowsOnlyQuit = true;
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
