using CommandLineParser;
namespace DefaultParsers
{
    /// <summary>
    /// Provides a parser that uses a delegate to convert strings to a specified type.
    /// </summary>
    /// <typeparam name="T">The type to parse from a string.</typeparam>
    public sealed class LambdaParser<T> : IParser<T>
    {
        /// <summary>
        /// Initializes a new instance of the LambdaParser class with a specified parsing function.
        /// </summary>
        /// <param name="parseFunc">A function that parses a string into a value of type T.</param>
        public LambdaParser(Func<string, T> parseFunc) 
        { 
            _parseFunc = parseFunc; 
        }
        
        /// <summary>
        /// Converts the specified string representation to its equivalent value of type T.
        /// </summary>
        /// <param name="value">The string to convert.</param>
        /// <returns>The value of type T equivalent to the specified string.</returns>
        public T Parse(string value) 
        { 
            return _parseFunc(value); 
        }

        private readonly Func<string, T> _parseFunc;
    }

    /// <summary>
    /// Provides functionality to parse strings into integer values.
    /// </summary>
    public sealed class IntParser : IParser<int>
    {
        /// <summary>
        /// Converts the specified string representation of a number to its 32-bit signed integer equivalent.
        /// </summary>
        /// <param name="value">A string containing a number to convert.</param>
        /// <returns>The 32-bit signed integer equivalent of the specified string.</returns>
        public int Parse(string value) 
        { 
            return int.Parse(value); 
        }
    }

    /// <summary>
    /// Provides functionality to parse strings without conversion.
    /// </summary>
    public sealed class StringParser : IParser<string>
    {
        /// <summary>
        /// Parses the specified string value and returns the result.
        /// </summary>
        /// <param name="value">The string to parse.</param>
        /// <returns>A string representing the parsed result.</returns>
        public string Parse(string value) 
        { 
            return value; 
        }
    }

    /// <summary>
    /// Provides functionality to parse string representations into boolean values.
    /// </summary>
    public sealed class BoolParser : IParser<bool>
    {
        /// <summary>
        /// Converts the specified string representation to its Boolean equivalent.
        /// </summary>
        /// <param name="value">The string to convert.</param>
        /// <returns>true if value is equivalent to Boolean true; otherwise, false.</returns>
        public bool Parse(string value) 
        { 
            return bool.Parse(value); 
        }
    }

    /// <summary>
    /// Provides parsing functionality for enum types.
    /// </summary>
    /// <typeparam name="T">The enum type to parse.</typeparam>
    public sealed class EnumParser<T> : IParser<T> where T : Enum
    {
        /// <summary>
        /// Converts the specified string representation to its equivalent value of type T.
        /// </summary>
        /// <param name="value">The string to convert.</param>
        /// <returns>The value of type T equivalent to the specified string.</returns>
        public T Parse(string value) 
        { 
            return (T)Enum.Parse(typeof(T), value, ignoreCase: true); 
        }
    }
}