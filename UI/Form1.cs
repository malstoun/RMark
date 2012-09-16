using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.IO;
using System.IO.Pipes;
using System.Net;
using System.Diagnostics;
using System.Net.Sockets;
using System.Windows.Forms;

using AForge.Video;
using AForge.Video.DirectShow;

namespace Limin
{
    public partial class Form1 : Form
    {
        public delegate void UploadImage(string s);

        bool isRunWatch;
        IAsyncResult res;
        FilterInfoCollection videoDevices;

        public Form1()
        {
            InitializeComponent();
            isRunWatch = false;
            videoDevices = new FilterInfoCollection(FilterCategory.VideoInputDevice);
        }

        private void открытьВидеопотокToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RunWatchStream();
        }

        private void RunWatchStream()
        {
            try
            { 
                if (videoDevices.Count > 0)
                {
                    VideoCaptureDevice vSource1 = new VideoCaptureDevice(videoDevices[0].MonikerString);

                    vSource1.DesiredFrameRate = 20;
                    vSource1.DesiredFrameSize = this.videoSourcePlayer1.Size;

                    videoSourcePlayer1.VideoSource = vSource1;
                    videoSourcePlayer1.Start();
                    isRunWatch = true;
                }
                if (videoDevices.Count == 2)
                {
                    VideoCaptureDevice vSource2 = new VideoCaptureDevice(videoDevices[1].MonikerString);

                    vSource2.DesiredFrameRate = 20;
                    vSource2.DesiredFrameSize = this.videoSourcePlayer1.Size;

                    videoSourcePlayer2.VideoSource = vSource2;
                    videoSourcePlayer2.Start();
                }
            }
            catch
            {
                return;
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            StopWatchStream();
        }

        private void StopWatchStream()
        {
            if (isRunWatch)
            {
                videoSourcePlayer1.SignalToStop();
                videoSourcePlayer1.WaitForStop();
                if (videoDevices.Count == 2)
                {
                    videoSourcePlayer2.SignalToStop();
                    videoSourcePlayer2.WaitForStop();
                }
            }
        }

        private void выходToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void загрузитьИзображенияToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fb = new FolderBrowserDialog();
            Timer timerExecute = new Timer();
            if (fb.ShowDialog() == DialogResult.OK)
            {
                UploadImage procImage = new UploadImage(FromFolder);
                res = procImage.BeginInvoke(fb.SelectedPath, null, null);
                загрузитьИзображенияToolStripMenuItem.Enabled = false;
                timerExecute.Interval = 1500;
                timerExecute.Tick += timerExecute_Tick;
                timerExecute.Start();
            }
        }

        void timerExecute_Tick(object sender, EventArgs e)
        {
            if (res.IsCompleted)
            {
                загрузитьИзображенияToolStripMenuItem.Enabled = true;
                ((Timer)(sender)).Stop();
                ((Timer)(sender)).Dispose();
            }
        }

        private void FromFolder(string s)
        {
            string[] files;
            byte[] buffer = new byte[255];
            byte[] buffer1;
            Encoding enc = Encoding.UTF8;

            Process.Start(vars.VARS.ServerExeFile, vars.VARS.ServerExeParams);

            NamedPipeClientStream client = new NamedPipeClientStream(".", "Limpipe", PipeDirection.Out, PipeOptions.None);
            NamedPipeClientStream clientRegNum = new NamedPipeClientStream(".", "LimpipeReg", PipeDirection.In, PipeOptions.None);
            client.Connect();
            clientRegNum.Connect();

            files = Directory.GetFiles(s);
            for (int i = 0; i < files.Length; i++)
            {
                if (files[i][files[i].Length - 1] != 'p' || files[i][files[i].Length - 2] != 'm')
                {
                    continue;
                }

                // Переводим в байты полученную картинку
                buffer1 = BmpToByteArray(files[i]);

                client.Write(buffer1, 0, buffer1.Length);
                client.WaitForPipeDrain();
                clientRegNum.Read(buffer, 0, 50);
                pictureBox1.ImageLocation = "\\\\psf\\Home\\Documents\\Visual Studio 2010\\Projects\\Limin\\Limin\\bin\\Debug\\" + (enc.GetString(buffer)).Replace("\0", "");
            }
            client.Close();
            client.Dispose();
            clientRegNum.Close();
            clientRegNum.Dispose();
        }

        private byte[] BmpToByteArray(string s)
        {
            Bitmap bmp = new Bitmap(s);

            // Создаем прямоугольную область и лочим в ней пиксели
            Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);
            System.Drawing.Imaging.BitmapData bmpData =
                bmp.LockBits(rect, System.Drawing.Imaging.ImageLockMode.ReadWrite,
                bmp.PixelFormat);

            // Получаем адрес первой строки
            IntPtr ptr = bmpData.Scan0;

            // Создаем массив для байтов пикселей
            int bytes  = Math.Abs(bmpData.Stride) * bmp.Height;
            byte[] rgbValues = new byte[bytes + 4];

            // Играемся со сдвигами, чтобы передать двухбайтное число (высоту/ширину) по одному байту

            rgbValues[0] = (byte)(bmp.Width >> 8);
            rgbValues[1] = (byte)((bmp.Width << 24) >> 24);
            rgbValues[2] = (byte)(bmp.Height >> 8);
            rgbValues[3] = (byte)((bmp.Height << 24) >> 24);

            // Копируем байты в массив
            System.Runtime.InteropServices.Marshal.Copy(ptr, rgbValues, 4, bytes);

            // Разлочка, которая может и не нужна
            bmp.UnlockBits(bmpData);

            bmp.Dispose();

            return rgbValues;
        }

        private void настройкиToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Dev_settings windowSettings = new Dev_settings();
            windowSettings.ShowDialog();
        }
    }
}
