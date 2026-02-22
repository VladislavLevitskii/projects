namespace FIRE_SIMULATION
{
    partial class UI
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            callGame = new Button();
            LName = new Label();
            LRules = new LinkLabel();
            timer_forest = new System.Windows.Forms.Timer(components);
            speed = new NumericUpDown();
            LFire = new Label();
            BEnd = new Button();
            BBack = new Button();
            timer_firefighter = new System.Windows.Forms.Timer(components);
            FCapacity = new ProgressBar();
            LCapacity = new Label();
            BFill = new Button();
            CDifficulty = new ComboBox();
            ((System.ComponentModel.ISupportInitialize)speed).BeginInit();
            SuspendLayout();
            // 
            // callGame
            // 
            callGame.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            callGame.Location = new Point(271, 741);
            callGame.Name = "callGame";
            callGame.RightToLeft = RightToLeft.No;
            callGame.Size = new Size(300, 100);
            callGame.TabIndex = 0;
            callGame.Text = "Play";
            callGame.UseVisualStyleBackColor = true;
            callGame.Click += callGame_Click;
            // 
            // LName
            // 
            LName.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            LName.Font = new Font("Segoe UI", 24F, FontStyle.Regular, GraphicsUnit.Point, 0);
            LName.Location = new Point(12, 90);
            LName.Name = "LName";
            LName.Size = new Size(858, 89);
            LName.TabIndex = 1;
            LName.Text = "Fire simulation";
            LName.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // LRules
            // 
            LRules.AutoSize = true;
            LRules.Font = new Font("Segoe UI", 18F, FontStyle.Regular, GraphicsUnit.Point, 204);
            LRules.Location = new Point(398, 179);
            LRules.Name = "LRules";
            LRules.Size = new Size(81, 41);
            LRules.TabIndex = 3;
            LRules.TabStop = true;
            LRules.Text = "rules";
            LRules.LinkClicked += LRules_LinkClicked;
            // 
            // timer_forest
            // 
            timer_forest.Tick += timer1_Tick;
            // 
            // speed
            // 
            speed.Increment = new decimal(new int[] { 100, 0, 0, 0 });
            speed.Location = new Point(12, 779);
            speed.Maximum = new decimal(new int[] { 1000, 0, 0, 0 });
            speed.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            speed.Name = "speed";
            speed.Size = new Size(150, 27);
            speed.TabIndex = 4;
            speed.Value = new decimal(new int[] { 1, 0, 0, 0 });
            speed.ValueChanged += numericUpDown1_ValueChanged;
            // 
            // LFire
            // 
            LFire.AutoSize = true;
            LFire.BackColor = Color.Red;
            LFire.Font = new Font("Segoe UI", 36F, FontStyle.Regular, GraphicsUnit.Point, 204);
            LFire.Location = new Point(26, 763);
            LFire.Name = "LFire";
            LFire.Size = new Size(164, 81);
            LFire.TabIndex = 5;
            LFire.Text = "Fire !";
            // 
            // BEnd
            // 
            BEnd.AutoSize = true;
            BEnd.Font = new Font("Segoe UI", 16.2F, FontStyle.Regular, GraphicsUnit.Point, 204);
            BEnd.Location = new Point(656, 779);
            BEnd.Name = "BEnd";
            BEnd.Size = new Size(193, 48);
            BEnd.TabIndex = 6;
            BEnd.Text = "Konec";
            BEnd.UseVisualStyleBackColor = true;
            BEnd.Click += BEnd_Click;
            // 
            // BBack
            // 
            BBack.AutoSize = true;
            BBack.Font = new Font("Segoe UI", 16.2F, FontStyle.Regular, GraphicsUnit.Point, 204);
            BBack.Location = new Point(656, 779);
            BBack.Name = "BBack";
            BBack.Size = new Size(202, 48);
            BBack.TabIndex = 8;
            BBack.Text = "Zpět do menu";
            BBack.UseVisualStyleBackColor = true;
            BBack.Click += BZpet_Click;
            // 
            // timer_firefighter
            // 
            timer_firefighter.Tick += timer_firefighter_Tick;
            // 
            // FCapacity
            // 
            FCapacity.Location = new Point(232, 763);
            FCapacity.Maximum = 20;
            FCapacity.Name = "FCapacity";
            FCapacity.Size = new Size(377, 29);
            FCapacity.Step = 1;
            FCapacity.TabIndex = 9;
            FCapacity.Value = 20;
            // 
            // LCapacity
            // 
            LCapacity.AutoSize = true;
            LCapacity.Font = new Font("Segoe UI", 16.2F, FontStyle.Regular, GraphicsUnit.Point, 204);
            LCapacity.Location = new Point(232, 803);
            LCapacity.Name = "LCapacity";
            LCapacity.Size = new Size(187, 38);
            LCapacity.TabIndex = 10;
            LCapacity.Text = "Kapacita vody";
            // 
            // BFill
            // 
            BFill.Font = new Font("Segoe UI", 13.8F, FontStyle.Regular, GraphicsUnit.Point, 204);
            BFill.Location = new Point(476, 800);
            BFill.Name = "BFill";
            BFill.Size = new Size(133, 41);
            BFill.TabIndex = 11;
            BFill.Text = "Naplnit";
            BFill.UseVisualStyleBackColor = true;
            BFill.Click += BFill_Click;
            // 
            // CDifficulty
            // 
            CDifficulty.DropDownStyle = ComboBoxStyle.DropDownList;
            CDifficulty.FormattingEnabled = true;
            CDifficulty.Items.AddRange(new object[] { "Lehká", "Střední", "Těžká" });
            CDifficulty.Location = new Point(271, 690);
            CDifficulty.Name = "CDifficulty";
            CDifficulty.Size = new Size(300, 28);
            CDifficulty.TabIndex = 12;
            // 
            // UI
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(882, 853);
            Controls.Add(CDifficulty);
            Controls.Add(BFill);
            Controls.Add(LCapacity);
            Controls.Add(FCapacity);
            Controls.Add(BBack);
            Controls.Add(BEnd);
            Controls.Add(LFire);
            Controls.Add(speed);
            Controls.Add(LRules);
            Controls.Add(LName);
            Controls.Add(callGame);
            KeyPreview = true;
            Name = "UI";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Forest simulation";
            ((System.ComponentModel.ISupportInitialize)speed).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button callGame;
        private Label LName;
        private LinkLabel LRules;
        private System.Windows.Forms.Timer timer_forest;
        private NumericUpDown speed;
        private Label LFire;
        private Button BEnd;
        private Button BBack;
        private System.Windows.Forms.Timer timer_firefighter;
        private ProgressBar FCapacity;
        private Label LCapacity;
        private Button BFill;
        private ComboBox CDifficulty;
    }
}
