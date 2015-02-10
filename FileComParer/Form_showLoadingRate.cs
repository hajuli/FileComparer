using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace FileComparer
{
    public partial class Form_showLoadingRate : Form
    {
        public bool cancelClicked = false;
        public Form_showLoadingRate()
        {
            InitializeComponent();
        }

        private void button_cancelLoading_Click(object sender, EventArgs e)
        {
            cancelClicked = true;
            this.Close();
        }

        public void setLoadingRate(int rate)
        {
            progressBar_loadingRate.Maximum = 100;
            progressBar_loadingRate.Value = rate;
        }

        public void setLoadingPath(string path)
        {
            if ("" != path)
            {
                label_loadingText.Text = "loading: " + path;
            }
        }
    }
}
