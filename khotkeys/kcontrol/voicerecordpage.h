/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffart  < ogoffart @ kde.org >

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef VOICE_RECORD_PAGE_H
#define VOICE_RECORD_PAGE_H

#include <tqvbox.h>



class TQWidget;
class TQPushButton;
class TQLabel;
class KLineEdit;



namespace KHotKeys
{

class Voice;
class VoiceRecorder;
class VoiceSignature;

class VoiceRecordPage : public QVBox
    {
    Q_OBJECT

    public:
        VoiceRecordPage(const TQString &voiceip_P, TQWidget *parent, const char *name);
        ~VoiceRecordPage();

        TQString getVoiceId() const ;
		VoiceSignature getVoiceSignature(int) const;
		bool isModifiedSignature(int) const;

    protected slots:
        void slotChanged();

    signals:
        void voiceRecorded(bool);

    private:
		VoiceRecorder *_recorder1;
		VoiceRecorder *_recorder2;
		KLineEdit *_lineEdit;
		TQLabel *_label;
		TQString _message;
		
		TQString _original_voiceId;
		
		
    };

} // namespace KHotKeys

#endif
