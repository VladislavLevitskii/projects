using CommandLineParser;

namespace CommandLineParser
{
    /// <summary>
    /// Defines a mechanism for validating string values.
    /// </summary>
    public interface IValidator<T>
    {
        /// <summary>
        /// Determines whether the specified string meets validation criteria.
        /// </summary>
        /// <param name="value">The value to validate.</param>
        /// <returns>true if the value is valid; otherwise, false.</returns>
        public bool Validate(T value);
    }
}