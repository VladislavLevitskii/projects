namespace CommandLineParser
{
    /// <summary>
    /// Defines a parser that converts a string to an instance of type T.
    /// </summary>
    /// <typeparam name="T">The type to parse from a string.</typeparam>
    public interface IParser<T>
    {
        /// <summary>
        /// Converts the specified string to an instance of type T.
        /// </summary>
        /// <param name="value">The string representation to convert.</param>
        /// <returns>An instance of type T equivalent to the specified string.</returns>
        public T Parse(string value);
    }
}