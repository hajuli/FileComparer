namespace FileComparer
{
    partial class Form_showLoadingRate
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.label_loadingText = new System.Windows.Forms.Label();
            this.progressBar_loadingRate = new System.Windows.Forms.ProgressBar();
            this.button_cancelLoading = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.panel1.Controls.Add(this.label_loadingText);
            this.panel1.Controls.Add(this.progressBar_loadingRate);
            this.panel1.Controls.Add(this.button_cancelLoading);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(634, 220);
            this.panel1.TabIndex = 0;
            // 
            // label_loadingText
            // 
            this.label_loadingText.AutoSize = true;
            this.label_loadingText.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_loadingText.Location = new System.Drawing.Point(26, 27);
            this.label_loadingText.Name = "label_loadingText";
            this.label_loadingText.Size = new System.Drawing.Size(62, 18);
            this.label_loadingText.TabIndex = 2;
            this.label_loadingText.Text = "loading";
            // 
            // progressBar_loadingRate
            // 
            this.progressBar_loadingRate.Location = new System.Drawing.Point(29, 68);
            this.progressBar_loadingRate.Name = "progressBar_loadingRate";
            this.progressBar_loadingRate.Size = new System.Drawing.Size(574, 34);
            this.progressBar_loadingRate.TabIndex = 1;
            // 
            // button_cancelLoading
            // 
            this.button_cancelLoading.Location = new System.Drawing.Point(453, 148);
            this.button_cancelLoading.Name = "button_cancelLoading";
            this.button_cancelLoading.Size = new System.Drawing.Size(117, 29);
            this.button_cancelLoading.TabIndex = 0;
            this.button_cancelLoading.Text = "Cancel";
            this.button_cancelLoading.UseVisualStyleBackColor = true;
            this.button_cancelLoading.Click += new System.EventHandler(this.button_cancelLoading_Click);
            // 
            // Form_showLoadingRate
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.GrayText;
            this.ClientSize = new System.Drawing.Size(634, 220);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "Form_showLoadingRate";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Form_showLoadingRate";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button button_cancelLoading;
        private System.Windows.Forms.ProgressBar progressBar_loadingRate;
        private System.Windows.Forms.Label label_loadingText;
    }
}