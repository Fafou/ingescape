

#Mastic.h
header_source_path = $$shell_path($$clean_path($$absolute_path($$PWD/../../../../src/include/mastic.h)))
header_dest_path = $$shell_path($$clean_path("C:/mastic/include/"))

#Create a command, using the 'cmd' command line and Window's 'xcopy'
copyHeaders.commands = $$quote(cmd /c xcopy /S /Y /I $${header_source_path} $${header_dest_path})

#dll
win32:{
    dependencies_src_path = $$shell_path($$clean_path($$absolute_path( $${libs_path}/*)))
    mastic_lib_src_path = $$shell_path($$clean_path($$absolute_path( $$DESTDIR/mastic.*)))

    CONFIG(debug, debug|release){
        dependencies_dest_path = $$shell_path($$clean_path("C:/mastic/libs/debug/*"))
    }else {
        dependencies_dest_path = $$shell_path($$clean_path("C:/mastic/libs/release/*"))
    }

    copyDependencies.commands = $$quote(cmd /c xcopy /S /Y /I $${dependencies_src_path} $${dependencies_dest_path})
    copyMasticLib.commands = $$quote( cmd /c xcopy /S /Y /I $${mastic_lib_src_path} $${dependencies_dest_path} )

    #Add the command to Qt.
    QMAKE_EXTRA_TARGETS += copyHeaders \
                           copyDependencies \
                           copyMasticLib \

    POST_TARGETDEPS += copyHeaders \
                       copyDependencies \
                       copyMasticLib \
}

unix:{
    android_compilation{
        dependencies_src_path = $$shell_path($$clean_path($$absolute_path( $${libs_path}/lib*)))
        mastic_lib_src_path = $$shell_path($$clean_path($$absolute_path( $$OUT_PWD/libmastic.so)))

        message($$mastic_lib_src_path)

        dependencies_dest_path = $$shell_path($$clean_path("C:/mastic/libs/android/armeabi-v7a/"))

        copyDependencies.commands = $$quote(cmd /c xcopy /S /Y /I $${dependencies_src_path} $${dependencies_dest_path})
        copyMasticLib.commands = $$quote( cmd /c xcopy /Y $${mastic_lib_src_path} $${dependencies_dest_path} )

        #Add the command to Qt.
        QMAKE_EXTRA_TARGETS += copyHeaders \
                               copyDependencies \
                               copyMasticLib \

        POST_TARGETDEPS += copyHeaders \
                           copyDependencies \
                           copyMasticLib \
    }
}
