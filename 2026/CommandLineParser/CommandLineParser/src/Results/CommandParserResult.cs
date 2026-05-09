using System;
using System.Linq;

using CommandLineParser.Exceptions;

namespace CommandLineParser
{
    /// <summary>
    /// Represents the result of parsing a command line, including validation status and access to parsed options and
    /// exceptions.
    /// </summary>
    /// <remarks>Use this type to inspect the outcome of command line parsing, retrieve options, and check for
    /// errors.</remarks>
    public sealed class CommandParserResult
    {
        internal CommandParserResult(
            Dictionary<OptionDefinition, CommandParserOptionResult> options,
            Dictionary<IParameterDefinition, object> plainArgs,
            Exception? error,
            string[] rawArgs)
        {
            _options = options;
            _plainArgs = plainArgs;
            _error = error;
            _rawArgs = rawArgs;
        }

        /// <summary>
        /// Converts a CommandParserResult to a Boolean indicating whether the result is valid.
        /// </summary>
        /// <param name="result">The command parser result to convert.</param>
        public static implicit operator bool(CommandParserResult result) => result.IsValid;

        /// <summary>
        /// Indicates whether the current instance is in a valid state.
        /// </summary>
        public bool IsValid => _error == null;

        /// <summary>
        /// Retrieves the first exception encountered during parsing.
        /// </summary>
        /// <returns>The first exception that occurred during parsing.</returns>
        public CommandLineParserException? GetException() 
        { 
            if (_error == null) return null;

            return _error as CommandLineParserException 
                ?? new CommandLineParserException(_error.Message, _error);
        }

        /// <summary>
        /// Retrieves the raw command-line string used for parsing.
        /// </summary>
        /// <returns>The full command-line string assembled from the parsed arguments.</returns>
        public string GetRawCommandLine()
        {
            return string.Join(" ", _rawArgs);
        }

        /// <summary>
        /// Retrieves the result for the specified command-line option.
        /// </summary>
        /// <param name="optionName">The name or alias of the option to retrieve.</param>
        /// <returns>The option result, or <see langword="null"/> if no matching option exists.</returns>
        public CommandParserOptionResult? GetOption(string optionName) 
        {
            var matchedDefinition = _options.Keys.FirstOrDefault(def => 
                def.GetName() == optionName || def.Aliases.Contains(optionName));

            return matchedDefinition != null ? _options[matchedDefinition] : null;
        }
        
        /// <summary>
        /// Retrieves the result for the specified option definition.
        /// </summary>
        /// <param name="option">The option definition to retrieve.</param>
        /// <returns>The option result, or <see langword="null"/> if the option was not parsed.</returns>
        public CommandParserOptionResult? GetOption(OptionDefinition option) 
        { 
            return _options.TryGetValue(option, out var result) ? result : null;
        }

        /// <summary>
        /// Determines whether the specified option is present.
        /// </summary>
        /// <param name="optionName">The name or alias of the option to check.</param>
        /// <returns><see langword="true"/> if the option was parsed; otherwise, <see langword="false"/>.</returns>
        public bool HasOption(string optionName) 
        { 
            return GetOption(optionName) != null;
        }

        /// <summary>
        /// Determines whether the specified option is present.
        /// </summary>
        /// <param name="option">The option definition to check.</param>
        /// <returns><see langword="true"/> if the option was parsed; otherwise, <see langword="false"/>.</returns>
        public bool HasOption(OptionDefinition option) 
        { 
            return _options.ContainsKey(option);
        }
        
        /// <summary>
        /// Retrieves a parsed plain argument by definition.
        /// </summary>
        /// <typeparam name="T">The type of the plain argument.</typeparam>
        /// <param name="def">The plain argument definition.</param>
        /// <returns>The parsed plain argument value, or <see langword="default"/> if it is unavailable.</returns>
        public T GetPlainArg<T>(ParameterDefinition<T> def) where T : notnull
        {
            var entry = _plainArgs.FirstOrDefault(x => x.Key.GetName() == def.Name);
            
            if (entry.Key != null && entry.Value != null)
            {
                return (T)entry.Value; 
            }
            
            return default!;
        }

        private readonly Dictionary<OptionDefinition, CommandParserOptionResult> _options;
        private readonly Dictionary<IParameterDefinition, object> _plainArgs;
        private readonly Exception? _error;
        private readonly string[] _rawArgs;

        /// <summary>
        /// Gets the parsing exception, if any.
        /// </summary>
        public Exception? Error => _error;
    }
}