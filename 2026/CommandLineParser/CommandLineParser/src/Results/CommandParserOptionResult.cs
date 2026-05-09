using CommandLineParser.Exceptions;
using Microsoft.VisualBasic;
using System.Linq;

namespace CommandLineParser
{
    /// <summary>
    /// Represents the result of parsing command-line options, including validation status and access to parsed
    /// arguments.
    /// </summary>
    public sealed class CommandParserOptionResult
    {

        /// <summary>
        /// Initializes a new instance of the CommandParserOptionResult class.
        /// </summary>
        /// <param name="definition">The option definition associated with the result.</param>
        /// <param name="parameters">The parsed parameter values.</param>
        /// <param name="parseError">The parsing error, if any.</param>
        internal CommandParserOptionResult(
            OptionDefinition definition, 
            Dictionary<string, object> parameters,
            Exception? parseError)
        {
            _definition = definition;
            _parameters = parameters;
            _parseError = parseError;
        }

        /// <summary>
        /// Converts a CommandParserOptionResult to a Boolean indicating whether the option is valid and present in the result.
        /// </summary>
        /// <param name="result">The command parser option result to convert.</param>
        public static implicit operator bool(CommandParserOptionResult result) 
        { 
            return result != null && result.IsValid;
        }

        /// <summary>
        /// Indicates whether the current instance is in a valid state.
        /// </summary>
        public bool IsValid => _parseError == null;

        /// <summary>
        /// Retrieves the value of the specified parameter and converts it to the specified type.
        /// </summary>
        /// <typeparam name="T">The type to convert the parameter value to.</typeparam>
        /// <param name="parameterName">The name of the parameter to retrieve.</param>
        /// <returns>The value of the parameter converted to type T, or <see langword="default"/> if it is unavailable.</returns>
        public T? GetArg<T>(string parameterName) 
        { 
            if (_parameters.TryGetValue(parameterName, out object? value) && value is T castedValue)
            {
                return castedValue;
            }
            
            return default;
        }

        /// <summary>
        /// Retrieves the value of the specified parameter.
        /// </summary>
        /// <typeparam name="T">The type of the parameter value.</typeparam>
        /// <param name="parameter">The parameter definition to retrieve the value for.</param>
        /// <returns>The value of the specified parameter, or <see langword="default"/> if it is unavailable.</returns>
        public T? GetArg<T>(ParameterDefinition<T> parameter) where T : notnull
        { 
            string paramName = ((IParameterDefinition)parameter).GetName();
            return GetArg<T>(paramName);
        }

        private readonly OptionDefinition _definition;
        private readonly Dictionary<string, object> _parameters;
        private readonly Exception? _parseError;
    }
}