using System;
using System.ComponentModel;
using System.Windows.Forms.Design;

namespace FIRE_SIMULATION
{

    public class Forest
    {
        private UI ui;
        public const int SIZE = 30;
        public const int CELLSIZE = 25;
        public static int capacity;
        public static int chance_fire_every_turn;
        public static int chance_grow_tree_every_turn;
        public static int chance_grow_fire_every_turn;
        public static int chance_die_fire;
        public static int speed_grow;
        public Cell[,] Grid { get; set; }

        public Forest(UI uiInstance)
        {
            ui = uiInstance;
            Grid = new Cell[SIZE, SIZE];
            Empty(Grid);
        }

        public void Empty(Cell[,] Grid)
        {
            for (int x = 0; x < SIZE; x++)
            {
                for (int y = 0; y < SIZE; y++)
                {
                    Grid[x, y] = new Cell();
                }
            }
        }

        public void GenerujLes()
        {
            Grid = new Cell[SIZE, SIZE];
            Empty(Grid);
            for (int i = 0; i < 10; i++)
            {
                int random_x = ui.rnd.Next(0, SIZE);
                int random_y = ui.rnd.Next(0, SIZE);
                int random_color = ui.rnd.Next(155, 256);
                Grid[random_x, random_y].State = Cell.CellState.Tree;
                Grid[random_x, random_y].ColorValue = random_color;
            }
            for (int i = 0; i < 1; i++)
            {
                FireFighter.FireFighters.Add(new FireFighter($"Hasič {i}", capacity));
            }
        }

        public void CalculatePath(Queue<int[]> Queue_Complement, int cellX, int cellY, FireFighter ff)
        {
            int nowX, nowY;

            if (ff.Queue_FF.Count > 0)
            {
                int[] posledni = ff.Queue_FF.Last();
                nowX = posledni[0];
                nowY = posledni[1];
            }
            else if (ff.Position != null)
            {
                nowX = ff.Position[0];
                nowY = ff.Position[1];
            }
            else
            {
                nowX = 0;
                nowY = 0;
            }

            while (nowX != cellX)
            {
                if (nowX > cellX)
                    nowX--;
                else
                    nowX++;

                Queue_Complement.Enqueue(new int[] { nowX, nowY });
            }

            while (nowY != cellY)
            {
                if (nowY > cellY)
                    nowY--;
                else
                    nowY++;

                Queue_Complement.Enqueue(new int[] { nowX, nowY });
            }
        }

        public void GenerateFire()
        {
            for (int x = 0; x < SIZE; x++)
            {
                for (int y = 0; y < SIZE; y++)
                {
                    if (Grid[x, y].State == Cell.CellState.Tree)
                    {
                        int chance = ui.rnd.Next(1, 1001);
                        if (chance < 5)
                        {
                            Grid[x, y].State = Cell.CellState.Burning;
                        }
                    }
                }
            }
        }

        public void UpdateState()
        {
            int[] dx = { -1, 0, 1, -1, 1, -1, 0, 1 };
            int[] dy = { -1, -1, -1, 0, 0, 1, 1, 1 };
            Cell[,] newGrid = new Cell[SIZE, SIZE];
            for (int x = 0; x < SIZE; x++)
            {
                for (int y = 0; y < SIZE; y++)
                {
                    List<FireFighter> fireFightersCopy = null;
                    if (Grid[x, y].FireFighters != null)
                    {
                        fireFightersCopy = new List<FireFighter>(Grid[x, y].FireFighters);
                    }
                    newGrid[x, y] = new Cell
                    {
                        State = Grid[x, y].State,
                        ColorValue = Grid[x, y].ColorValue,
                        FireFighters = fireFightersCopy
                    };
                }
            }


            if (ui.state == UI.State.GAME)
            {
                int chance = ui.rnd.Next(0, 101);
                if (chance < chance_fire_every_turn)
                {
                    ui.ChooseState(UI.State.FIRE);
                    return;
                } else
                {
                    for (int y = 0; y < SIZE; y++)
                    {
                        for (int x = 0; x < SIZE; x++)
                        {
                            if (Grid[x, y].ColorValue >= 50)
                            {
                                int count = 0;
                                for (int i = 0; i < 8; i++)
                                {
                                    int nx = x + dx[i], ny = y + dy[i];
                                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && Grid[nx, ny].State == Cell.CellState.Tree)
                                    {
                                        count += 1;
                                    }
                                }

                                newGrid[x, y].ColorValue -= (count) / speed_grow + 1;
                            }

                            if (Grid[x, y].State == Cell.CellState.Empty)
                            {
                                int count = 0;
                                for (int i = 0; i < 8; i++)
                                {
                                    int nx = x + dx[i], ny = y + dy[i];
                                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && Grid[nx, ny].State == Cell.CellState.Tree)
                                    {
                                        count += 1;
                                    }
                                }
                                int chance_grow = ui.rnd.Next(1, 1001);
                                if (chance_grow < chance_grow_tree_every_turn*count)
                                {
                                    newGrid[x, y].State = Cell.CellState.Tree;
                                    newGrid[x, y].ColorValue = 255;
                                }
                            }
                        }
                    }
                }
            }

            else if (ui.state == UI.State.FIRE) { // chovani pozar

                for (int y = 0; y < SIZE; y++)
                {
                    for (int x = 0; x < SIZE; x++)
                    {
                        if (Grid[x, y].State == Cell.CellState.Burning)
                        {
                            int count = 0;
                            for (int i = 0; i < 8; i++)
                            {
                                int nx = x + dx[i], ny = y + dy[i];
                                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && Grid[nx, ny].State == Cell.CellState.Tree)
                                {
                                    count += 1;
                                }
                            }
                            int chance_die = ui.rnd.Next(0, 101);
                            if (chance_die < chance_die_fire || count == 0)
                            {
                                newGrid[x, y].State = Cell.CellState.Empty;
                            } else
                            {
                                for (int i = 0; i < 8; i++)
                                {
                                    int nx = x + dx[i], ny = y + dy[i];
                                    if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && Grid[nx, ny].State == Cell.CellState.Tree)
                                    {
                                        int chance_grow = ui.rnd.Next(1, 101);
                                        if (chance_grow < chance_grow_fire_every_turn)
                                        {
                                            newGrid[nx, ny].State = Cell.CellState.Burning;
                                            newGrid[nx, ny].ColorValue = 0;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }

            Grid = newGrid;
        }
    }
}