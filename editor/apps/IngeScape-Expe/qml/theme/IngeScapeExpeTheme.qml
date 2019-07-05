/*
 *	IngeScape Expe
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

pragma Singleton
import QtQuick 2.9
import I2Quick 1.0


/**
  * Singleton that provides access to standard colors, fonts, etc.
  */
Item {
    id: theme


    //-----------------------------------------
    //
    // Sizes of UI elements
    //
    //-----------------------------------------

    // Minimum size of our application (Window)
    readonly property int appMinWidth: 1024
    readonly property int appMinHeight: 768

    // Default size of our application (Window)
    readonly property int appDefaultWidth: 1920
    readonly property int appDefaultHeight: 1080


    //-----------------------------------------
    //
    // SVG files
    //
    //-----------------------------------------

    // - Main SVG file
    /*property var svgFileIngeScapeExpe : svgFileCacheIngeScapeExpe

    I2SvgFileCache {
        id: svgFileCacheIngeScapeExpe
        svgFile: "qrc:/resources/SVG/ingescape-expe.svg"
    }*/


    //-----------------------------------------
    //
    // Colors
    //
    //-----------------------------------------



    //-----------------------------------------
    //
    // Fonts
    //
    //-----------------------------------------

}
