namespace fire_detect_gui
{
    partial class Form1
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.chioce_file_btn = new System.Windows.Forms.Button();
            this.image = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.result = new System.Windows.Forms.Label();
            this.status = new System.Windows.Forms.Label();
            this.status_text = new System.Windows.Forms.Label();
            this.result_text = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.real_time_btn = new System.Windows.Forms.Button();
            this.start_btn = new System.Windows.Forms.Button();
            this.stop_btn = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.image)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // chioce_file_btn
            // 
            this.chioce_file_btn.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold);
            this.chioce_file_btn.Location = new System.Drawing.Point(23, 407);
            this.chioce_file_btn.Name = "chioce_file_btn";
            this.chioce_file_btn.Size = new System.Drawing.Size(114, 54);
            this.chioce_file_btn.TabIndex = 0;
            this.chioce_file_btn.Text = "选择文件";
            this.chioce_file_btn.UseVisualStyleBackColor = true;
            this.chioce_file_btn.Click += new System.EventHandler(this.Choice_file_btn_Click);
            // 
            // image
            // 
            this.image.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.image.Location = new System.Drawing.Point(401, 37);
            this.image.Name = "image";
            this.image.Size = new System.Drawing.Size(672, 581);
            this.image.TabIndex = 4;
            this.image.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.result);
            this.groupBox1.Controls.Add(this.status);
            this.groupBox1.Controls.Add(this.status_text);
            this.groupBox1.Controls.Add(this.result_text);
            this.groupBox1.Location = new System.Drawing.Point(12, 37);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(383, 337);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "数据框";
            // 
            // result
            // 
            this.result.AutoSize = true;
            this.result.Font = new System.Drawing.Font("宋体", 16F, System.Drawing.FontStyle.Bold);
            this.result.Location = new System.Drawing.Point(228, 220);
            this.result.Name = "result";
            this.result.Size = new System.Drawing.Size(96, 27);
            this.result.TabIndex = 9;
            this.result.Text = "无结果";
            // 
            // status
            // 
            this.status.AutoSize = true;
            this.status.Font = new System.Drawing.Font("宋体", 16F, System.Drawing.FontStyle.Bold);
            this.status.Location = new System.Drawing.Point(228, 49);
            this.status.Name = "status";
            this.status.Size = new System.Drawing.Size(124, 27);
            this.status.TabIndex = 8;
            this.status.Text = "等待检测";
            // 
            // status_text
            // 
            this.status_text.AutoSize = true;
            this.status_text.Font = new System.Drawing.Font("宋体", 16F, System.Drawing.FontStyle.Bold);
            this.status_text.Location = new System.Drawing.Point(6, 49);
            this.status_text.Name = "status_text";
            this.status_text.Size = new System.Drawing.Size(124, 27);
            this.status_text.TabIndex = 7;
            this.status_text.Text = "检测状态";
            // 
            // result_text
            // 
            this.result_text.AutoSize = true;
            this.result_text.Font = new System.Drawing.Font("宋体", 16F, System.Drawing.FontStyle.Bold);
            this.result_text.Location = new System.Drawing.Point(6, 220);
            this.result_text.Name = "result_text";
            this.result_text.Size = new System.Drawing.Size(124, 27);
            this.result_text.TabIndex = 7;
            this.result_text.Text = "检测结果";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(398, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(67, 15);
            this.label1.TabIndex = 6;
            this.label1.Text = "实时图像";
            // 
            // real_time_btn
            // 
            this.real_time_btn.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold);
            this.real_time_btn.Location = new System.Drawing.Point(245, 407);
            this.real_time_btn.Name = "real_time_btn";
            this.real_time_btn.Size = new System.Drawing.Size(114, 54);
            this.real_time_btn.TabIndex = 7;
            this.real_time_btn.Text = "实时检测";
            this.real_time_btn.UseVisualStyleBackColor = true;
            this.real_time_btn.Click += new System.EventHandler(this.Real_time_btn_Click);
            // 
            // start_btn
            // 
            this.start_btn.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold);
            this.start_btn.Location = new System.Drawing.Point(23, 529);
            this.start_btn.Name = "start_btn";
            this.start_btn.Size = new System.Drawing.Size(114, 54);
            this.start_btn.TabIndex = 8;
            this.start_btn.Text = "开始检测";
            this.start_btn.UseVisualStyleBackColor = true;
            this.start_btn.Click += new System.EventHandler(this.Start_btn_Click);
            // 
            // stop_btn
            // 
            this.stop_btn.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold);
            this.stop_btn.Location = new System.Drawing.Point(245, 529);
            this.stop_btn.Name = "stop_btn";
            this.stop_btn.Size = new System.Drawing.Size(114, 54);
            this.stop_btn.TabIndex = 9;
            this.stop_btn.Text = "停止检测";
            this.stop_btn.UseVisualStyleBackColor = true;
            this.stop_btn.Click += new System.EventHandler(this.Stop_btn_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1085, 630);
            this.Controls.Add(this.stop_btn);
            this.Controls.Add(this.start_btn);
            this.Controls.Add(this.real_time_btn);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.image);
            this.Controls.Add(this.chioce_file_btn);
            this.Name = "Form1";
            ((System.ComponentModel.ISupportInitialize)(this.image)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button chioce_file_btn;
        private System.Windows.Forms.PictureBox image;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label result;
        private System.Windows.Forms.Label status;
        private System.Windows.Forms.Label status_text;
        private System.Windows.Forms.Label result_text;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button real_time_btn;
        private System.Windows.Forms.Button start_btn;
        private System.Windows.Forms.Button stop_btn;
    }
}

