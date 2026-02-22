using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace FIRE_SIMULATION
{
    public class FireFighter
    {
        public string Name { get; set; }
        public int Capacity { get; set; }
        public Queue<int[]> Queue_FF { get; set; }
        public int[] Position { get; set; }

        public bool Disappear { get; set; }

        public FireFighter(string name, int capacity)
        {
            Name = name;
            Capacity = capacity;
            Queue_FF = new Queue<int[]>();
            Disappear = false;
        }
        public static List<FireFighter> FireFighters { get; } = new List<FireFighter>();

        public void FillWater(FireFighter fireFighter)
        {
            fireFighter.Queue_FF.Clear();
            if (fireFighter.Position !=  null)
            {
                int x = fireFighter.Position[0];
                int y = fireFighter.Position[1];
                int leftDist = x;
                int rightDist = 29 - x;
                int topDist = y;
                int bottomDist = 29 - y;
                int min = Math.Min(Math.Min(leftDist, rightDist), Math.Min(topDist, bottomDist));

                int dx = 0, dy = 0;
                if (min == leftDist) dx = -1;
                else if (min == rightDist) dx = 1;
                else if (min == topDist) dy = -1;
                else if (min == bottomDist) dy = 1;

                while (x >= 0 && x < 30 && y >= 0 && y < 30)
                {
                    fireFighter.Queue_FF.Enqueue(new int[] { x, y });
                    x += dx;
                    y += dy;
                }
                fireFighter.Disappear = true;
            }
        }
    }

}
