using System.Drawing.Imaging;

namespace FIRE_SIMULATION
{
    public class ForestRenderer
    {
        private static Bitmap bmp;
        private static Graphics g;
        private static readonly Pen GridPen = Pens.Black;
        private static readonly Brush BurningBrush = Brushes.Red;
        private static readonly Brush EmptyBrush = Brushes.LightGray;

        public static Bitmap Render(Cell[,] grid)
        {
            int sizePx = Forest.SIZE * Forest.CELLSIZE + 1;

            if (bmp == null)
            {
                bmp = new Bitmap(sizePx, sizePx, PixelFormat.Format24bppRgb);
                g = Graphics.FromImage(bmp);
                g.SetClip(new Rectangle(0, 0, sizePx, sizePx));
            }

            g.Clear(Color.White);

            var burningRects = new List<Rectangle>(Forest.SIZE * Forest.SIZE);
            var emptyRects = new List<Rectangle>(Forest.SIZE * Forest.SIZE);
            var greenRects = new List<Rectangle>(Forest.SIZE * Forest.SIZE);
            var greenBrushes = new List<Brush>(Forest.SIZE * Forest.SIZE);

            for (int y = 0; y < Forest.SIZE; y++)
            {
                for (int x = 0; x < Forest.SIZE; x++)
                {
                    var rect = new Rectangle(x * Forest.CELLSIZE, y * Forest.CELLSIZE,
                                             Forest.CELLSIZE, Forest.CELLSIZE);
                    var cell = grid[x, y];

                    switch (cell.State)
                    {
                        case Cell.CellState.Burning:
                            burningRects.Add(rect);
                            break;
                        case Cell.CellState.Empty:
                            emptyRects.Add(rect);
                            break;
                        case Cell.CellState.Tree:
                            greenRects.Add(rect);
                            greenBrushes.Add(new SolidBrush(Color.FromArgb(0, cell.ColorValue, 0)));
                            break;
                    }
                }
            }

            if (burningRects.Count > 0)
                g.FillRectangles(BurningBrush, burningRects.ToArray());
            if (emptyRects.Count > 0)
                g.FillRectangles(EmptyBrush, emptyRects.ToArray());
            for (int i = 0; i < greenRects.Count; i++)
            {
                g.FillRectangle(greenBrushes[i], greenRects[i]);
                greenBrushes[i].Dispose();
            }

            int end = Forest.SIZE * Forest.CELLSIZE;
            for (int i = 0; i <= Forest.SIZE; i++)
            {
                int pos = i * Forest.CELLSIZE;
                g.DrawLine(GridPen, pos, 0, pos, end);
                g.DrawLine(GridPen, 0, pos, end, pos);
            }

            for (int y = 0; y < Forest.SIZE; y++)
            {
                for (int x = 0; x < Forest.SIZE; x++)
                {
                    var cell = grid[x, y];
                    if (cell.FireFighters != null && cell.FireFighters.Count > 0)
                    {
                        int px = x * Forest.CELLSIZE;
                        int py = y * Forest.CELLSIZE;
                        int r = Forest.CELLSIZE / 2 + 5;

                        g.FillEllipse(Brushes.Blue,
                                      px + (Forest.CELLSIZE - r) / 2,
                                      py + (Forest.CELLSIZE - r) / 2,
                                      r, r);
                    }
                }
            }

            return bmp;
        }
    }
}
