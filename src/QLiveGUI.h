/*
 *  QLiveGUI.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires ciUI : https://github.com/rezaali/ciUI
 *
 */


#ifndef QLIVE_GUI
#define QLIVE_GUI

#pragma once

#include "cinder/app/AppBasic.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "QLive.h"
#include "ciUI.h"


namespace nocte {
    
    class QLiveGUI {
        
    public:
        
        QLiveGUI( QLive *live, int offsetX = 0 ) : mLive(live), mOffsetX(offsetX)
        {
            mGUI = NULL;
            init();
        }
        
        
        ~QLiveGUI()
        {
            if ( mGUI )
                delete mGUI;
        }
        
        
        void update() 
        { 
            if ( mGUI ) 
                mGUI->update(); 
        }
        
        
        void render() 
        { 
            if ( mGUI ) 
                mGUI->draw(); 
        }
        
        
        void init() 
        {
            Vec2f mModuleGUISize(130, 300);
            
            QLiveTrack      *track;
            QLiveClip       *clip;
            QLiveDevice     *device;
            QLiveParam      *param;
            
            std::vector<QLiveTrack*> tracks = mLive->getTracks();
            
            float               w       = mModuleGUISize.x - CI_UI_GLOBAL_WIDGET_SPACING * 2;
            float               h       = 10.0f;
            Vec2i               pos     = Vec2i( CI_UI_GLOBAL_WIDGET_SPACING, CI_UI_GLOBAL_WIDGET_SPACING );
            ciUIWidget          *widget;
            ciUILabel           *labelWidget;
            
            mIsFullWidth = false;
            
            if ( mGUI )
                delete mGUI;

            mGUI = new ciUICanvas( 0, 0, ci::app::getWindowWidth() - mOffsetX, mModuleGUISize.y );

            mGUI->setFont(RES_GUI_FONT);
            mGUI->setFontSize( CI_UI_FONT_LARGE, 14 );
            mGUI->setFontSize( CI_UI_FONT_MEDIUM, 12 );
            mGUI->setFontSize( CI_UI_FONT_SMALL, 10 );
            mGUI->setTheme( CI_UI_THEME_NOCTE_GREEN );
            
            std::vector<QLiveDevice*>   devices;
            std::vector<QLiveClip*>     clips;
            std::vector<QLiveParam*>    params;
            std::vector<std::string>    clipNames;
            
        
            for( int k=0; k < tracks.size(); k++ )
            {
                track   = tracks[k];
             
                if ( boost::starts_with( track->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
                    continue;
                
                pos.y   = CI_UI_GLOBAL_WIDGET_SPACING;
                
                // Label
                widget = new ciUILabel( pos.x, pos.y, track->getName(), CI_UI_FONT_MEDIUM );
                mGUI->addWidget( widget );
                widget->setColorFill( track->getColor() );
                widget->setColorOutline( track->getColor() );
                pos.y += widget->getRect()->getHeight() + 3;

                // Volume                
                widget = new ciUISlider( pos.x, pos.y, w, h, 0, 1.0, track->getVolumeRef(), "Master" );
                widget->setColorFill( track->getColor() );
                widget->setMeta( toString(k) );
                
                mGUI->addWidget( widget );
                pos.y += 24;
                
                mGUI->addWidget( new ciUISpacer( pos.x, pos.y, w, 1 ) );
                pos.y += 6;
                
                // Clips
                clips = track->getClips();
                
                for( int i=0; i < clips.size(); i++ )
                {
                    clip    = clips[i];
                    widget  = new ciUIToggle( pos.x, pos.y, 8, 8, clip->getIsPlayingRef(), clip->getName(), CI_UI_FONT_SMALL );
                    widget->setMeta( "clip_" + toString( track->getIndex() ) + "_" + toString( clip->getIndex() ) );
                    mGUI->addWidget( widget );
                    widget->setColorFill( clip->getColor() );
                    labelWidget = ((ciUIWidgetWithLabel*)widget)->getLabelWidget();
                    labelWidget->setColorFill( clip->getColor() );
                    ((ciUIToggle*)widget)->setLabelOffset( 0, 8 );
                    pos.y += widget->getRect()->getHeight() + 4;
                }
                pos.y += 3;
                
                mGUI->addWidget( new ciUISpacer( pos.x, pos.y, w, 1 ) );
                pos.y += 7;
                
                // Params
                devices = track->getDevices();
                
                for( int i=0; i < devices.size(); i++ )
                {
                    device = devices[i];
                    params = device->getParams();
                    
                    for( int j=1; j < params.size(); j++ )       // starts from 1 to ignore "Device On"
                    {
                        param = params[j];
                        widget = new ciUISlider( pos.x, pos.y, w, h/2.0f, param->getMin(), param->getMax(), param->getRef(), param->getName() );
                        widget->setMeta( "param_" + toString( track->getIndex() ) + "_" + toString( device->getIndex() ) + "_" + toString( param->getIndex() ) );
                        
                        mGUI->addWidget( widget );
                        pos.y += widget->getRect()->getHeight() * 4.5f + 3;
                    }
                }                
                
                pos.x   += CI_UI_GLOBAL_WIDGET_SPACING + mModuleGUISize.x;
            }
            
            mGUI->autoSizeToFitWidgets();   
            
            ciUIRectangle* rect = mGUI->getRect();
            rect->setHeight( rect->getHeight() + CI_UI_GLOBAL_WIDGET_SPACING*2.5f );
            rect->setX( mOffsetX );
            rect->setY( ci::app::getWindowHeight() - rect->getHeight() );
            
            if ( rect->getWidth() < ci::app::getWindowWidth() - mOffsetX )
                rect->setWidth( ci::app::getWindowWidth() - mOffsetX );
            
            mGUI->registerUIEvents(this, &QLiveGUI::guiEvent);
        }
        
        
        void guiEvent(ciUIEvent *event)
        {
            std::string name = event->widget->getName();
            std::string meta = event->widget->getMeta();

            if(name == "Master")
            {
                int trackIdx = boost::lexical_cast<int>( event->widget->getMeta() );
                ciUISlider *slider = (ciUISlider *) event->widget;
                mLive->setTrackVolume( trackIdx, slider->getScaledValue() );
            }
            
            else if ( boost::find_first( meta, "clip") )
            {
                ciUIToggle *toggle = (ciUIToggle *) event->widget;
                
                std::vector<std::string>    splitValues;                
                std::string                 clipMeta;
                bool                        toggleVal   = toggle->getValue();
                
                boost::split( splitValues, meta, boost::is_any_of("_") );
                
                int trackIdx    = boost::lexical_cast<int>( splitValues[1] );
                int clipIdx     = boost::lexical_cast<int>( splitValues[2] );
                
                toggle->setValue( !toggle->getValue() );    // trigger back the toogle, QLive sets the value, this is to avoid flickering
                
                if ( toggleVal )
                    mLive->playClip( trackIdx, clipIdx );               // play clip
                else
                    mLive->stopClip( trackIdx, clipIdx );               // play clip
            }
            
            else if ( boost::find_first( meta, "param") )
            {
                // param_TRACKIDX_DEVICEIDX_PARAMIDX
                
                ciUISlider *slider = (ciUISlider *) event->widget;
                
                std::vector<std::string> splitValues;
                boost::split( splitValues, meta, boost::is_any_of("_") );
                
                int trackIdx    = boost::lexical_cast<int>( splitValues[1] );
                int deviceIdx   = boost::lexical_cast<int>( splitValues[2] );
                int paramIdx    = boost::lexical_cast<int>( splitValues[3] );

                mLive->setParam( trackIdx, deviceIdx, paramIdx, slider->getScaledValue() );
            }
            
        }
        
        
        void toggleVisible() { mGUI->toggleVisible(); };
        
        
        void toggleFullWidth() 
        { 
            mIsFullWidth = !mIsFullWidth; 
            
            ciUIRectangle* rect = mGUI->getRect();
            
            if ( mIsFullWidth )
            {
                rect->setX( 0 );
                rect->setWidth( ci::app::getWindowWidth() );
            }
            else
            {
                mGUI->autoSizeToFitWidgets();
                rect->setX( mOffsetX );
                rect->setY( ci::app::getWindowHeight() - rect->getHeight() );
                
                if ( rect->getWidth() < ci::app::getWindowWidth() - mOffsetX )
                    rect->setWidth( ci::app::getWindowWidth() - mOffsetX );
            }
            
        }
        
        
        void shutdown()
        {
            if ( mGUI )
                delete mGUI;
        }
        
    private:
        
        QLive       *mLive;
        ciUICanvas  *mGUI;
        int         mOffsetX;
        bool        mIsFullWidth;

    };
    
}

#endif