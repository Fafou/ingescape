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

import INGESCAPE 1.0



Item {
    id: root



    //
    // Background
    //
    Rectangle {
        id: background

        anchors.fill: parent

        gradient: Gradient {
            GradientStop { position: 0.0; color: IngeScapeTheme.loadingScreen_lightBlue }
            GradientStop { position: 1.0; color: IngeScapeTheme.loadingScreen_darkBlue }
        }
    }



    //
    // Animated logo
    //
    Item {
        id: animatedLogo

        anchors.centerIn: parent

        width: 297
        height: 297

        I2SvgItem {
            id: logo

            svgFileCache: IngeScapeAssessmentsTheme.svgFileIngeScapeAssessments
            svgElementId: "logo-loader"
        }

        RotationAnimator {
            target: logo

            from: 0
            to: 360

            duration: 2200
            loops: Animation.Infinite
            running: root.visible
        }
    }



    //
    // Title
    //
    Column {

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: animatedLogo.bottom
            topMargin: 50
        }

        spacing: 20


        // Label: IngeScape Assessments
        Row {

            Text {
                anchors {
                    bottom: parent.bottom
                }

                text: qsTr("INGE")

                font {
                    family: IngeScapeTheme.loadingFontFamily
                    pointSize: 101
                }

                color: "#6D6E70"
            }

            Text {
                anchors {
                    bottom: parent.bottom
                }

                text: qsTr("SCAPE ")

                font {
                    family: IngeScapeTheme.loadingFontFamily
                    pointSize: 101
                }

                color: "#B5B7B6"
            }

            Text {
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 8
                }

                text: qsTr("ASSESSMENTS")

                font {
                    family: IngeScapeTheme.loadingFontFamily
                    pointSize: 68
                }

                color: "#B5B7B6"
            }
        }


        // Version
        Text {
            id: version

            anchors.horizontalCenter: parent.horizontalCenter

            text: qsTr("VERSION %1").arg(Qt.application.version)

            font {
                family: IngeScapeTheme.loadingFontFamily
                pointSize: 21
            }

            color: "#6D6E70"
        }
    }
}
