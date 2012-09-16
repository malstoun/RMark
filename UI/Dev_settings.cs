using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Limin
{
    public partial class Dev_settings : Form
    {
        public Dev_settings()
        {
            InitializeComponent();
            textBox1.Text = vars.VARS.ServerExeFile;
            textBox2.Text = vars.VARS.ServerExeParams;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            OpenFileDialog fd = new OpenFileDialog();
            if (fd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                textBox1.Text = fd.FileName;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            vars.VARS.ServerExeFile = textBox1.Text;
            vars.VARS.ServerExeParams = textBox2.Text;
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
        }
    }
}
