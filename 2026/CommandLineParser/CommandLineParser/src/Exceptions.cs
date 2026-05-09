namespace CommandLineParser.Exceptions
{
    /// <summary>
    /// Base exception for all exceptions thrown by the CommandLineParser library.
    /// </summary>
    public class CommandLineParserException : Exception
    {
        /// <summary>Initializes a new instance of the <see cref="CommandLineParserException"/> class.</summary>
        public CommandLineParserException() { }
        /// <summary>Initializes a new instance with a specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public CommandLineParserException(string message) : base(message) { }
        /// <summary>Initializes a new instance with a specified error message and a reference to the inner exception.</summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="inner">The exception that is the cause of the current exception.</param>
        public CommandLineParserException(string message, Exception inner) : base(message, inner) { }
    }
    /// <summary>
    /// Exception thrown when duplicate option aliases are detected.
    /// </summary>
    public class CommandLineParserAliasException : CommandLineParserException
    {
        /// <summary>Initializes a new instance of the <see cref="CommandLineParserAliasException"/> class.</summary>
        public CommandLineParserAliasException() { }
        /// <summary>Initializes a new instance with a specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public CommandLineParserAliasException(string message) : base(message) { }
        /// <summary>Initializes a new instance with a specified error message and a reference to the inner exception.</summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="inner">The exception that is the cause of the current exception.</param>
        public CommandLineParserAliasException(string message, Exception inner) : base(message, inner) { }
    }
    /// <summary>
    /// Exception thrown when a user-defined parameter validation fails during command-line parsing.
    /// </summary>
    public class CommandLineParserValidationException : CommandLineParserException
    {
        /// <summary>Initializes a new instance of the <see cref="CommandLineParserValidationException"/> class.</summary>
        public CommandLineParserValidationException() { }
        /// <summary>Initializes a new instance with a specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public CommandLineParserValidationException(string message) : base(message) { }
        /// <summary>Initializes a new instance with a specified error message and a reference to the inner exception.</summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="inner">The exception that is the cause of the current exception.</param>
        public CommandLineParserValidationException(string message, Exception inner) : base(message, inner) { }
    }
    /// <summary>
    /// Exception thrown when user-defined parameter parsing fails during command-line parsing.
    /// </summary>
    public class CommandLineParserParsingException : CommandLineParserException
    {
        /// <summary>Initializes a new instance of the <see cref="CommandLineParserParsingException"/> class.</summary>
        public CommandLineParserParsingException() { }
        /// <summary>Initializes a new instance with a specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public CommandLineParserParsingException(string message) : base(message) { }
        /// <summary>Initializes a new instance with a specified error message and a reference to the inner exception.</summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="inner">The exception that is the cause of the current exception.</param>
        public CommandLineParserParsingException(string message, Exception inner) : base(message, inner) { }
    }
}