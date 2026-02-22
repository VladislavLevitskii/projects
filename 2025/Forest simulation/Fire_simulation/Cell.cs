using System.Diagnostics.Tracing;

namespace FIRE_SIMULATION
{
    public class Cell
    {
        public enum CellState
        {
            Empty,
            Tree,
            Burning
        }

        public CellState State { get; set; }
        public int ColorValue { get; set; }
        public List<FireFighter> FireFighters { get; set; } = new List<FireFighter>();

        public Cell()
        {
            State = CellState.Empty;
            ColorValue = 0;
            FireFighters = new List<FireFighter>();
        }
    }
}