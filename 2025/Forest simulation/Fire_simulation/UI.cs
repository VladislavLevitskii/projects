using System;
using System.Diagnostics;
using System.DirectoryServices.ActiveDirectory;
using System.Drawing;
using System.Runtime.CompilerServices;
using System.Security.Policy;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace FIRE_SIMULATION
{

    public partial class UI : Form
    {

        public State state;
        public enum State { MENU, GAME, FIRE, END }

        private Forest forest;
        private Bitmap forestBmp;
        private DoubleBufferPanel panel;
        private bool first_time = true;
        public Random rnd = new Random();
        public List<int> count_trees = new List<int>();

        public UI()
        {
            InitializeComponent();
            forest = new Forest(this);
            ChooseState(State.MENU);
            LRules.Links.Add(0, 100, "https://es.wikipedia.org/wiki/Memoria_(juego)");
            timer_forest.Tick += timer1_Tick;
            this.KeyPreview = true;
            this.KeyDown += new KeyEventHandler(UI_KeyDown);
        }

        private void UI_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                BFill.Focus();
                BFill.PerformClick();
            }
        }

        public void ChooseState(State newState)
        {
            switch (newState)
            {
                case State.MENU:
                    FireFighter.FireFighters.Clear();
                    Cell[,] grid = new Cell[Forest.SIZE, Forest.SIZE];
                    forest.Empty(grid);
                    forestBmp = ForestRenderer.Render(grid);
                    callGame.Visible = true;
                    LName.Visible = true;
                    LRules.Visible = true;
                    speed.Visible = false;
                    LFire.Visible = false;
                    BEnd.Visible = false;
                    BBack.Visible = false;
                    LFire.Text = "Fire !";
                    if (panel != null)
                    {
                        panel.Visible = false;
                    }
                    count_trees.Clear();
                    EmptyFF();
                    FCapacity.Visible = false;
                    LCapacity.Visible = false;
                    BFill.Visible = false;
                    CDifficulty.Visible = true;
                    CDifficulty.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
                    CDifficulty.SelectedIndex = 0;
                    break;
                case State.GAME:
                    CDifficulty.Visible = false;
                    BEnd.Visible = true;
                    BBack.Visible = false;
                    callGame.Visible = false;
                    LName.Visible = false;
                    LRules.Visible = false;
                    speed.Visible = false;
                    LFire.Visible = false;
                    if (first_time)
                    {
                        GeneratePanel();
                        first_time = false;
                        FCapacity.Value = FCapacity.Maximum;
                    }
                    panel.Visible = true;
                    timer_forest.Interval = 1;
                    timer_forest.Start();
                    timer_firefighter.Interval = 50;
                    timer_firefighter.Start();
                    LCapacity.Visible = true;
                    FCapacity.Visible = true;
                    BFill.Visible = true;
                    BFill.Focus();
                    break;
                case State.FIRE:
                    timer_forest.Interval = 1000;
                    LFire.Visible = true;
                    BBack.Visible = false;
                    forest.GenerateFire();
                    break;
                case State.END:
                    BEnd.Visible = false;

                    DrawChart();

                    BBack.Visible = true;
                    break;
            }
            state = newState;
        }

        private void callGame_Click(object sender, EventArgs e)
        {
            ChooseState(State.GAME);
        }

        public void DrawChart()
        {
            Form chartForm = new Form
            {
                Width = 600,
                Height = 400,
                Text = "Průběh počtu stromů"
            };

            var chart = new System.Windows.Forms.DataVisualization.Charting.Chart
            {
                Dock = DockStyle.Fill
            };
            var area = new ChartArea("MainArea");
            chart.ChartAreas.Add(area);

            var series = new Series("Počet stromů")
            {
                ChartType = SeriesChartType.Line,
                BorderWidth = 3,
                Color = Color.Red
            };

            int skore = 0;

            for (int i = 0; i < count_trees.Count; i++)
            {
                series.Points.AddXY(i, count_trees[i]);
                skore += count_trees[i];
            }
            chart.Series.Add(series);

            skore = skore / count_trees.Count;

            Label scoreLabel = new Label
            {
                Location = new Point(10, 10),
                AutoSize = true,
                Font = new Font("Segoe UI", 18),
                Text = $"Skóre: {skore}"
            };

            chartForm.Controls.Add(scoreLabel);
            chartForm.Controls.Add(chart);

            chartForm.StartPosition = FormStartPosition.Manual;
            chartForm.Location = new Point(
                this.Left + (this.Width - chartForm.Width) / 2,
                this.Top + (this.Height - chartForm.Height) / 2
            );

            chartForm.Show();

            if (series.Points.Last().YValues[0] == 0)
                LFire.Text = "Konec";
        }

        public void EmptyFF()
        {
            foreach (var hasic in FireFighter.FireFighters)
            {
                hasic.Queue_FF.Clear();
                hasic.Position = null;
            }
        }
        private void LRules_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start(new ProcessStartInfo
            {
                FileName = "https://gitlab.mff.cuni.cz/teaching/nprg031/2425-summer/student-levitsv",
                UseShellExecute = true
            });
        }

        private void Panel_MouseClick(object sender, MouseEventArgs e)
        {
            if (state == State.FIRE)
            {
                int cellX = e.X / Forest.CELLSIZE;
                int cellY = e.Y / Forest.CELLSIZE;

                if (cellX >= 0 && cellX < Forest.SIZE)
                {
                    var ff = FireFighter.FireFighters[0];
                    if (ff.Position != null)
                    {
                        Queue<int[]> Queue_complement = new Queue<int[]>();
                        forest.CalculatePath(Queue_complement, cellX, cellY, ff);
                        while (Queue_complement.Count > 0)
                        {
                            ff.Queue_FF.Enqueue(Queue_complement.Dequeue());
                        }
                    }
                    else
                    {
                        ff.Queue_FF.Enqueue([cellX, cellY]);
                    }
                }
            }
        }

        void GeneratePanel()
        {
            if (panel != null)
            {
                this.Controls.Remove(panel);
                panel.Dispose();
                panel = null;
            }

            panel = new DoubleBufferPanel();
            panel.BackgroundImage = forestBmp;
            panel.BackgroundImageLayout = ImageLayout.None;
            panel.Width = (Forest.SIZE * Forest.CELLSIZE + 1) - 0;
            panel.Height = (Forest.SIZE * Forest.CELLSIZE + 1) - 0;
            int x = (this.ClientSize.Width - panel.Width) / 2;
            int y = 0;
            panel.Location = new Point(x, y);
            panel.BackColor = Color.White;
            panel.MouseClick += Panel_MouseClick;
            this.Controls.Add(panel);

            forest.GenerujLes();

            forestBmp = ForestRenderer.Render(forest.Grid);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (state == State.GAME || state == State.FIRE)
            {
                forest.UpdateState();
                forestBmp = ForestRenderer.Render(forest.Grid);
                int count = 0;
                bool end = true;
                for (int x = 0; x < Forest.SIZE; x++)
                {
                    for (int y = 0; y < Forest.SIZE; y++)
                    {
                        if (forest.Grid[x, y].State == Cell.CellState.Tree)
                        {
                            count++;
                            end = false;
                        }
                    }
                }
                count_trees.Add(count);
                panel.BackgroundImage = forestBmp;
                panel.Invalidate();
                if (end)
                {
                    ChooseState(State.END);
                }
                if (state == State.FIRE)
                {
                    bool ohen_neni = true;
                    for (int x = 0; x < Forest.SIZE && ohen_neni; x++)
                    {
                        for (int y = 0; y < Forest.SIZE && ohen_neni; y++)
                        {
                            if (forest.Grid[x, y].State == Cell.CellState.Burning)
                            {
                                ohen_neni = false;
                                break;
                            }
                        }
                    }
                    if (ohen_neni)
                    {
                        ChooseState(State.GAME);
                    }
                }
            }
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            //timer_forest.Interval = (int)speed.Value;
        }

        private void BEnd_Click(object sender, EventArgs e)
        {
            ChooseState(State.END);
        }

        private void BZpet_Click(object sender, EventArgs e)
        {
            first_time = true;
            ChooseState(State.MENU);
        }

        private void timer_firefighter_Tick(object sender, EventArgs e)
        {
            if (state == State.FIRE)
            {
                foreach (var FireFighter in FireFighter.FireFighters)
                {
                    if (FireFighter.Queue_FF.Count != 0)
                    {
                        int[] kam = FireFighter.Queue_FF.Dequeue();

                        if (FireFighter.Position != null)
                        {
                            int prevX = FireFighter.Position[0];
                            int prevY = FireFighter.Position[1];
                            forest.Grid[prevX, prevY].FireFighters.Remove(FireFighter);
                        }

                        FireFighter.Position = kam;
                        forest.Grid[kam[0], kam[1]].FireFighters.Add(FireFighter);
                    }
                    if (FireFighter.Position != null && forest.Grid[FireFighter.Position[0], FireFighter.Position[1]].State == Cell.CellState.Burning && FireFighter.Capacity > 0)
                    {
                        forest.Grid[FireFighter.Position[0], FireFighter.Position[1]].State = Cell.CellState.Empty;
                        FireFighter.Capacity--;
                        FCapacity.Value = FireFighter.Capacity;
                    }
                    if (FireFighter.Position != null && FireFighter.Disappear && (FireFighter.Position[0] == 0 || FireFighter.Position[0] == 29 || FireFighter.Position[1] == 0 || FireFighter.Position[1] == 29))
                    {
                        forest.Grid[FireFighter.Position[0], FireFighter.Position[1]].FireFighters.Remove(FireFighter);
                        FireFighter.Disappear = false;
                        FCapacity.Value = FCapacity.Maximum;
                        FireFighter.Capacity = Forest.capacity;
                    }

                }
                forestBmp = ForestRenderer.Render(forest.Grid);
                panel.BackgroundImage = forestBmp;
                panel.Invalidate();
            }
        }

        private void BFill_Click(object sender, EventArgs e)
        {
            foreach (var FireFighter in FireFighter.FireFighters)
                FireFighter.FillWater(FireFighter);
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            int selectedIndex = CDifficulty.SelectedIndex;
            if (selectedIndex == 0)
            {
                Forest.capacity = 50;
                FCapacity.Maximum = 50;
                Forest.chance_fire_every_turn = 1;
                Forest.chance_grow_tree_every_turn = 8;
                Forest.speed_grow = 4;
                Forest.chance_die_fire = 10;
                Forest.chance_grow_fire_every_turn = 15;
            }
            else if (selectedIndex == 1)
            {
                Forest.capacity = 25;
                FCapacity.Maximum = 25;
                Forest.chance_fire_every_turn = 2;
                Forest.chance_grow_tree_every_turn = 4;
                Forest.speed_grow = 5;
                Forest.chance_die_fire = 5;
                Forest.chance_grow_fire_every_turn = 25;
            }
            else if (selectedIndex == 2)
            {
                Forest.capacity = 10;
                FCapacity.Maximum = 10;
                Forest.chance_fire_every_turn = 3;
                Forest.chance_grow_tree_every_turn = 2;
                Forest.speed_grow = 6;
                Forest.chance_die_fire = 2;
                Forest.chance_grow_fire_every_turn = 35;
            }
        }
    }

    public class DoubleBufferPanel : Panel
    {
        public DoubleBufferPanel()
        {
            SetStyle(ControlStyles.AllPaintingInWmPaint |
                     ControlStyles.OptimizedDoubleBuffer |
                     ControlStyles.UserPaint, true);
            UpdateStyles();
        }
    }
}