#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTDebug>
#include <QFileDialog>

#include "libltc/ltc.h"
#include "libltc/encoder.h"
#include "libltc/decoder.h"

enum SelectFrameRate {
    FPS24 = 0,
    FPS25 = 1,
    FPS2997 = 2,
    FPS30 = 3
};

enum SelectSampleRate {
    SAMPLERATE44100 = 0,
    SAMPLERATE48000 = 1
};

enum SelectSoundMode {
    MONO = 0,
    STEREO = 1
};

enum SelectBitRate {
    BIT8 = 0,
    BIT16 = 1
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    checkAndSetDropFrameDisabledState();
    checkAndSetMaxFrameNumber();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonSave_clicked()
{
    qDebug("starting save to wav");

    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save to WAV"), "", tr("WAV File (*.wav)"));

    if (saveFileName.isEmpty())
    {
        qDebug("no file specified");
        return;
    }

    SMPTETimecode fromTimecode = {0};
    fromTimecode.hours = ui->sbFromHours->value();
    fromTimecode.mins = ui->sbFromMins->value();
    fromTimecode.secs = ui->sbFromSecs->value();
    fromTimecode.frame = ui->sbFromFrames->value();

    SMPTETimecode toTimecode = {0};
    toTimecode.hours = ui->sbToHours->value();
    toTimecode.mins = ui->sbToMins->value();
    toTimecode.secs = ui->sbToSecs->value();
    toTimecode.frame = ui->sbToFrames->value();

    // how long?

    unsigned int fromFrames = static_cast<unsigned int>(((fromTimecode.hours * 60 * 60) + (fromTimecode.mins * 60) + (fromTimecode.secs)) * frameRate()) + fromTimecode.frame;
    unsigned int toFrames = static_cast<unsigned int>(((toTimecode.hours * 60 * 60) + (toTimecode.mins * 60) + (toTimecode.secs)) * frameRate()) + toTimecode.frame;

    if (toFrames < fromFrames)
    {
        qWarning("to cannot be before from time");
        return;
    }

    qint32 sampleRate = getSampleRate();
    qint16 numChannels = nChannels();
    int bitsPerSample = bitRate();

    // check to is after from

    LTCEncoder* encoder = ltc_encoder_create(
                sampleRate,
                frameRate(),
                (ui->comboFrameRate->currentIndex() == SelectFrameRate::FPS25 ? LTC_TV_625_50 : LTC_TV_525_60),
                0);

    if (encoder)
    {
        ltc_encoder_set_timecode(encoder, &fromTimecode);

        if (QFile::exists(saveFileName))
        {
            qDebug("file exists, overwritting");
            QFile::remove(saveFileName);
        }

        QFile wavFile(saveFileName);
        if (wavFile.open(QIODevice::WriteOnly))
        {
            QDataStream out(&wavFile);

            out.setByteOrder(QDataStream::LittleEndian);

            out.writeRawData("RIFF", 4);
            out << quint32(0); // chunk size
            out.writeRawData("WAVE", 4);

            out.writeRawData("fmt ", 4);
            out << quint32(16); // "fmt " chunk size (always 16 for PCM)
            out << quint16(1); // data format
            out << quint16(numChannels);
            out << quint32(sampleRate);
            out << quint32(sampleRate * numChannels * bitsPerSample / 8 ); // bytes per second
            out << quint16(numChannels * bitsPerSample / 8); // Block align
            out << quint16(bitsPerSample); // Significant Bits Per Sample

            out.writeRawData("data", 4);
            out << quint32(0);

            SMPTETimecode progressTimecode = {};
            unsigned int progressFrames = 0;

            qDebug("generating audio, writing data");

            ltcsnd_sample_t *buffer = new ltcsnd_sample_t[ltc_encoder_get_buffersize(encoder)];

            ltcsnd_sample_t *memoryBuffer8bit = nullptr;
            qint16 *memoryBuffer16bit = nullptr;

            if (bitsPerSample == 8)
                memoryBuffer8bit = new ltcsnd_sample_t[ltc_encoder_get_buffersize(encoder) * numChannels];
            else
                memoryBuffer16bit = new qint16[ltc_encoder_get_buffersize(encoder) * numChannels];

            while (progressFrames <= toFrames)
            {
                ltc_encoder_encode_frame(encoder);
                int nBytesToWrite = ltc_encoder_get_buffer(encoder, buffer);


                for (int i = 0; i < nBytesToWrite; ++i)
                {
                    for (int iChannel = 0; iChannel < numChannels; ++iChannel)
                    {
                        if (bitsPerSample == 8)
                            memoryBuffer8bit[(i * numChannels) + iChannel] = buffer[i];
                        else
                            memoryBuffer16bit[(i * numChannels) + iChannel] = (qint16(buffer[i]) - 128) << 8;
                    }
                }
                if (bitsPerSample == 8)
                    out.writeRawData((char*)memoryBuffer8bit, nBytesToWrite * numChannels);
                else
                    out.writeRawData((char*)memoryBuffer16bit, nBytesToWrite * numChannels * 2);

                ltc_encoder_inc_timecode(encoder);
                ltc_encoder_get_timecode(encoder, &progressTimecode);
                progressFrames = static_cast<unsigned int>(((progressTimecode.hours * 60 * 60) + (progressTimecode.mins * 60) + (progressTimecode.secs)) * frameRate()) + progressTimecode.frame;
            }

            delete buffer;
            delete memoryBuffer8bit;
            delete memoryBuffer16bit;

            qDebug("writing data complete");

            wavFile.flush();

            wavFile.seek(4);
            out << quint32(wavFile.size() - 8);

            // data chunk size
            wavFile.seek(40);
            out << quint32(wavFile.size() - 44);

            wavFile.flush();
            wavFile.close();
        }
        else
        {
            qWarning("failed to open file for writing");
        }

    }
    else
    {
        qCritical("failed to create encoder");
    }

    ltc_encoder_free(encoder);
}

void MainWindow::on_comboFrameRate_activated(int index)
{
    Q_UNUSED(index);

    checkAndSetDropFrameDisabledState();
    checkAndSetMaxFrameNumber();
}

void MainWindow::checkAndSetDropFrameDisabledState()
{
    switch(ui->comboFrameRate->currentIndex())
    {
    case SelectFrameRate::FPS24:
        ui->checkDropFrame->setDisabled(true);
        break;
    case SelectFrameRate::FPS25:
        ui->checkDropFrame->setDisabled(true);
        break;
    case SelectFrameRate::FPS2997:
        ui->checkDropFrame->setDisabled(false);
        break;
    case SelectFrameRate::FPS30:
        ui->checkDropFrame->setDisabled(true);
        break;
    }
}

void MainWindow::checkAndSetMaxFrameNumber()
{
    int maxFrameNumber = 0;

    switch(ui->comboFrameRate->currentIndex())
    {
    case SelectFrameRate::FPS24:
        maxFrameNumber = 24;
        break;
    case SelectFrameRate::FPS25:
        maxFrameNumber = 25;
        break;
    case SelectFrameRate::FPS2997:
        maxFrameNumber = 29;
        break;
    case SelectFrameRate::FPS30:
        maxFrameNumber = 29;
        break;
    }

    if (ui->sbFromFrames->value() > maxFrameNumber)
        ui->sbFromFrames->setValue(maxFrameNumber);

    if (ui->sbToFrames->value() > maxFrameNumber)
        ui->sbToFrames->setValue(maxFrameNumber);

    ui->sbFromFrames->setMaximum(maxFrameNumber);
    ui->sbToFrames->setMaximum(maxFrameNumber);
}

double MainWindow::frameRate()
{
    switch(ui->comboFrameRate->currentIndex())
    {
    case SelectFrameRate::FPS24:
        return 24.0;
        break;
    case SelectFrameRate::FPS25:
        return 25.0;
        break;
    case SelectFrameRate::FPS2997:
        return 29.97;
        break;
    case SelectFrameRate::FPS30:
        return 30.0;
        break;
    }
    qCritical("oops");
    return 0.0;
}

int MainWindow::baseFrameRate()
{
    switch(ui->comboFrameRate->currentIndex())
    {
    case SelectFrameRate::FPS24:
        return 24;
        break;
    case SelectFrameRate::FPS25:
        return 25;
        break;
    case SelectFrameRate::FPS2997:
        return 30;
        break;
    case SelectFrameRate::FPS30:
        return 30;
        break;
    }
    qCritical("oops");
    return 0;
}


int MainWindow::getSampleRate()
{
    switch(ui->comboSampleRate->currentIndex())
    {
    case SelectSampleRate::SAMPLERATE44100:
        return 44100;
        break;
    case SelectSampleRate::SAMPLERATE48000:
        return 48000;
        break;
    }
    return 0;
}


int MainWindow::bitRate()
{
    switch(ui->comboBitRate->currentIndex())
    {
    case SelectBitRate::BIT8:
        return 8;
        break;
    case SelectBitRate::BIT16:
        return 16;
        break;
    }
    return 0;
}

int MainWindow::nChannels()
{
    switch(ui->comboSoundMode->currentIndex())
    {
    case SelectSoundMode::MONO:
        return 1;
        break;
    case SelectSoundMode::STEREO:
        return 2;
        break;
    }
    return 0;
}
