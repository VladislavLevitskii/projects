global using static CommandLineParser.Constants;
using CommandLineParser.Exceptions;

namespace CommandLineParser
{
    internal static class Constants
    {
        public const string defaultPlainArgsDelimiter = "--";
        public const char dash = '-';
        public const char assignChar = '=';
        public const int columnWidthBase = 20;
        public const int columnGap = 4;
    }

    /// <summary>
    /// Provides a builder for configuring and creating command parsers with customizable options, arguments, and help providers.
    /// </summary>
    public sealed class CommandParserBuilder
    {
        /// <summary>
        /// Initializes a new instance of the CommandParserBuilder class with the specified delimiters.
        /// </summary>
        /// <param name="plainArgsDelimiter">The delimiter used to separate plain arguments.</param>
        public CommandParserBuilder(string plainArgsDelimiter = defaultPlainArgsDelimiter)
        {
            _plainArgsDelimiter = plainArgsDelimiter;
        }

        /// <summary>
        /// Configures the command parser to use the specified help provider.
        /// </summary>
        /// <param name="helpProvider">The help provider to use for generating help information.</param>
        /// <returns>The current instance of CommandParserBuilder.</returns>
        public CommandParserBuilder WithHelp(HelpProvider helpProvider)
        {
            _helpProvider = helpProvider;
            _helpProviderSet = true;
            return this;
        }

        /// <summary>
        /// This function is using factory design to create custom HelpProviders 
        /// </summary>
        /// <param name="factory">Function that will create custom HelpProvider</param>
        /// <returns>Updated CommandParserBuilder</returns>
        public CommandParserBuilder WithCustomHelp(Func<List<OptionHelpInfo>, List<ParameterHelpInfo>, ProgramHelpInfo, HelpProvider> factory)
        {
            _customHelpFactory = factory;
            return this;
        }

        /// <summary>
        /// Adds an option definition to the command parser builder.
        /// </summary>
        /// <param name="option">The option definition to add.</param>
        /// <returns>The updated CommandParserBuilder instance.</returns>
        public CommandParserBuilder WithOption(OptionDefinition option)
        {
            _options.Add(option);
            return this;
        }

        /// <summary>
        /// Adds a plain argument definition to the command parser builder.
        /// </summary>
        /// <typeparam name="T">The type of the argument value.</typeparam>
        /// <param name="plainArgDef">The definition of the plain argument to add.</param>
        /// <returns>The updated command parser builder instance.</returns>
        public CommandParserBuilder WithPlainArg<T>(ParameterDefinition<T> plainArgDef) where T : notnull
        {
            _plainArguments.Add(plainArgDef);
            return this;
        }

        /// <summary>
        /// Adds a plain argument to the command parser with the specified name and configuration.
        /// </summary>
        /// <typeparam name="Type">The type of the argument value.</typeparam>
        /// <param name="name">The name of the argument.</param>
        /// <param name="configure">A delegate to configure the argument definition.</param>
        /// <returns>The updated CommandParserBuilder instance.</returns>
        public CommandParserBuilder WithPlainArg<Type>(string name, Func<ParameterDefinitionBuilder<Type>, ParameterDefinition<Type>> configure) where Type : notnull
        {
            ParameterDefinitionBuilder<Type> builder = new(name);
            ParameterDefinition<Type> parameter = configure(builder);
            _plainArguments.Add(parameter);
            return this;
        }

        /// <summary>
        /// Creates and returns a new instance of the CommandParser.
        /// </summary>
        /// <returns>A CommandParser instance.</returns>
        public CommandParser Build()
        {
            CheckAliasDuplicity();
            CheckParameterDuplicity();

            HelpProvider finalProvider = SetFinalProvider();

            return new CommandParser(_options, _plainArguments, finalProvider, _plainArgsDelimiter);
        }

        private HelpProvider SetFinalProvider()
        {
            if (_customHelpFactory == null)
            {
                return _helpProviderSet ? _helpProvider : new DefaultHelpProvider(_options, _plainArguments, _helpProvider);
            } else
            {
                List<OptionHelpInfo> optionsInfo = _options.Select(o => {
                    var optionHelp = o.GetHelp();
                    
                    var nestedParameters = o.ParameterDefinitions.Select(p => {
                        var paramHelp = o.GetHelp();
                        return new ParameterHelpInfo(
                            p.GetName(), 
                            paramHelp.GetLongHelpText(), 
                            paramHelp.GetShortHelpText()
                        );
                    }).ToList().AsReadOnly();

                    return new OptionHelpInfo(
                        o._name, 
                        optionHelp.GetLongHelpText(), 
                        optionHelp.GetShortHelpText(),
                        o.Aliases,
                        nestedParameters
                    );
                }).ToList()!;

                List<ParameterHelpInfo> parametersInfo = _plainArguments.Select(p => {
                    var help = p.GetHelp();
                    return new ParameterHelpInfo(
                        p.GetName(), 
                        help.GetLongHelpText(), 
                        help.GetShortHelpText()
                    );
                }).ToList()!;

                ProgramHelpInfo programHelpInfo = new(_helpProvider.GetLongHelpText(), _helpProvider.GetShortHelpText());

                return _customHelpFactory(optionsInfo, parametersInfo, programHelpInfo)!;
            }
        }

        /// <summary>
        /// Creates and returns a new instance of the CommandParser and assigns it to the out parameter.
        /// </summary>
        /// <param name="parser">When this method returns, contains the newly created CommandParser instance.</param>
        /// <returns>The same instance of CommandParser that was created.</returns>
        public CommandParser Build(out CommandParser parser)
        {
            parser = Build();
            return parser;
        }

        internal void CheckAliasDuplicity()
        {
            var duplicate = _options
                .SelectMany(option => option.Aliases.Select(alias => new { alias, option }))
                .GroupBy(item => item.alias)
                .FirstOrDefault(group => group.Skip(1).Any());

            if (duplicate != null)
            {
                var first = duplicate.First();
                throw new CommandLineParserAliasException(
                    $"Alias duplicity: '{first.alias}' in '{first.option._name}' is duplicity and already exists");
            }
        }

        internal void CheckParameterDuplicity()
        {
            var duplicate = _options
                .SelectMany(option => option.ParameterDefinitions)
                .GroupBy(parameter => parameter.GetName())
                .FirstOrDefault(group => group.Skip(1).Any());

            if (duplicate != null)
            {
                throw new CommandLineParserException(
                    $"Parameter duplicity: '{duplicate.Key}' is duplicity and already exists");
            }
        }

        private readonly List<OptionDefinition> _options = new();
        private readonly List<IParameterDefinition> _plainArguments = new();
        private HelpProvider _helpProvider = new();
        private Func<List<OptionHelpInfo>, List<ParameterHelpInfo>, ProgramHelpInfo, HelpProvider>? _customHelpFactory;        
        private bool _helpProviderSet = false;
        private readonly string _plainArgsDelimiter;
    }

    /// <summary>
    /// Parses command-line input and provides access to defined options and arguments.
    /// </summary>
    public sealed class CommandParser
    {
        /// <summary>
        /// Initializes a new instance for parsing commands using specified delimiters and help provider.
        /// </summary>
        internal CommandParser(List<OptionDefinition> options, List<IParameterDefinition> plainArguments, HelpProvider helpProvider, string plainArgsDelimiter)
        {
            _options = options;
            _plainArguments = plainArguments;
            _helpProvider = helpProvider;
            _plainArgsDelimiter = plainArgsDelimiter;
        }

        /// <summary>
        /// Parses an array of command line arguments and returns the result.
        /// </summary>
        /// <param name="args">The parsed command line arguments.</param>
        /// <returns>A result object containing the parsed command information.</returns>
        public CommandParserResult Parse(string[] args)
        {
            ParseContext context = new(_options, _plainArguments, args, _plainArgsDelimiter);
            ParseCommandLine(context);

            ApplyDefaultOptionValues(context);
            CheckMissingPlainArguments(context, ref context.ParseError);
            CheckMissingRequiredOptions(context, ref context.ParseError);

            return new CommandParserResult(context.ParsedOptions, context.ParsedPlainArgs, context.ParseError, args);
        }

        private static void ParseCommandLine(ParseContext context)
        {
            OptionMatcher matcher = new(context.Options);
            OptionParameterBinder optionBinder = new();
            PlainArgumentBinder plainBinder = new(context.PlainArguments);

            for (int i = 0; i < context.Args.Length; i++)
            {
                string currentArg = context.Args[i];
                if (string.IsNullOrWhiteSpace(currentArg)) continue;

                if (!context.ForcePlainArgs && currentArg == context.PlainArgsDelimiter)
                {
                    context.ForcePlainArgs = true;
                    continue;
                }

                if (matcher.TryMatchOption(currentArg, context.ForcePlainArgs, out var matchedOption, out var inlineParamValue))
                {
                    ParseOption(context, matchedOption, inlineParamValue, ref i, optionBinder, currentArg);
                    continue;
                }

                plainBinder.BindPlainArgument(currentArg, context.ParsedPlainArgs, ref context.CurrentPlainArgIndex, ref context.ParseError);
            }
        }

        private static void ParseOption(
            ParseContext context,
            OptionDefinition matchedOption,
            string? inlineParamValue,
            ref int argIndex,
            OptionParameterBinder optionBinder,
            string currentArg)
        {
            if (context.ParsedOptions.ContainsKey(matchedOption))
            {
                context.ParseError ??= new CommandLineParserParsingException($"Option '{currentArg}' was provided multiple times.");
                return;
            }

            Dictionary<string, object> optionParameters = optionBinder.BindParameters(
                matchedOption,
                context.Args,
                ref argIndex,
                ref inlineParamValue,
                ref context.ParseError,
                out Exception? optionError);

            var optionResult = new CommandParserOptionResult(matchedOption, optionParameters, optionError);
            context.ParsedOptions[matchedOption] = optionResult;
            context.OptionOrder.Add(optionResult);
        }

        private static void ApplyDefaultOptionValues(ParseContext context)
        {
            DefaultOptionApplier applier = new(context.Options);
            applier.ApplyDefaults(context.ParsedOptions);
        }

        private static void CheckMissingPlainArguments(ParseContext context, ref Exception? parseError)
        {
            MissingRequirementsChecker checker = new(context.Options, context.PlainArguments);
            checker.CheckPlainArguments(context.CurrentPlainArgIndex, ref parseError);
        }

        private static void CheckMissingRequiredOptions(ParseContext context, ref Exception? parseError)
        {
            MissingRequirementsChecker checker = new(context.Options, context.PlainArguments);
            checker.CheckRequiredOptions(context.ParsedOptions, ref parseError);
        }

        /// <summary>
        /// Retrieves the configured help provider.
        /// </summary>
        /// <returns>The help provider used by this parser.</returns>
        public HelpProvider GetHelp() => _helpProvider;

        private readonly List<OptionDefinition> _options;
        private readonly List<IParameterDefinition> _plainArguments;
        private HelpProvider _helpProvider;
        private readonly string _plainArgsDelimiter;

        private sealed class ParseContext
        {
            public ParseContext(List<OptionDefinition> options, List<IParameterDefinition> plainArguments, string[] args, string plainArgsDelimiter)
            {
                Options = options;
                PlainArguments = plainArguments;
                Args = args;
                PlainArgsDelimiter = plainArgsDelimiter;
                ParsedOptions = new Dictionary<OptionDefinition, CommandParserOptionResult>();
                ParsedPlainArgs = new Dictionary<IParameterDefinition, object>();
                OptionOrder = new List<CommandParserOptionResult>();
            }

            public List<OptionDefinition> Options { get; }
            public List<IParameterDefinition> PlainArguments { get; }
            public string[] Args { get; }
            public string PlainArgsDelimiter { get; }
            public Dictionary<OptionDefinition, CommandParserOptionResult> ParsedOptions { get; }
            public Dictionary<IParameterDefinition, object> ParsedPlainArgs { get; }
            public List<CommandParserOptionResult> OptionOrder { get; }
            public Exception? ParseError;
            public bool ForcePlainArgs;
            public int CurrentPlainArgIndex;
        }

        private sealed class OptionMatcher
        {
            private readonly List<OptionDefinition> _options;

            public OptionMatcher(List<OptionDefinition> options)
            {
                _options = options;
            }

            public bool TryMatchOption(string currentArg, bool forcePlainArgs, out OptionDefinition matchedOption, out string? inlineParamValue)
            {
                matchedOption = null!;
                inlineParamValue = null;
                if (forcePlainArgs) return false;

                string searchAlias = currentArg;
                int equalsIndex = currentArg.IndexOf(assignChar);
                if (equalsIndex > 0)
                {
                    searchAlias = currentArg.Substring(0, equalsIndex);
                    inlineParamValue = currentArg.Substring(equalsIndex + 1);
                }

                matchedOption = _options.FirstOrDefault(o => o.Aliases.Contains(searchAlias))!;
                return matchedOption != null;
            }
        }

        private sealed class OptionParameterBinder
        {
            public Dictionary<string, object> BindParameters(
                OptionDefinition matchedOption,
                string[] args,
                ref int argIndex,
                ref string? inlineParamValue,
                ref Exception? parseError,
                out Exception? optionError)
            {
                Dictionary<string, object> optionParameters = new();
                optionError = null;

                foreach (var paramDef in matchedOption.ParameterDefinitions)
                {
                    string? paramValue = null;
                    if (inlineParamValue != null)
                    {
                        paramValue = inlineParamValue;
                        inlineParamValue = null;
                    }
                    else if (argIndex + 1 < args.Length && !args[argIndex + 1].StartsWith(dash))
                    {
                        paramValue = args[++argIndex];
                    }

                    try
                    {
                        object parsedValue = paramDef.GetParameter(paramValue);
                        optionParameters[paramDef.GetName()] = parsedValue;
                    }
                    catch (Exception ex)
                    {
                        optionError ??= ex;
                        parseError ??= ex;
                    }
                }

                return optionParameters;
            }
        }

        private sealed class PlainArgumentBinder
        {
            private readonly List<IParameterDefinition> _plainArguments;

            public PlainArgumentBinder(List<IParameterDefinition> plainArguments)
            {
                _plainArguments = plainArguments;
            }

            public void BindPlainArgument(
                string currentArg,
                Dictionary<IParameterDefinition, object> parsedPlainArgs,
                ref int currentPlainArgIndex,
                ref Exception? parseError)
            {
                if (currentPlainArgIndex >= _plainArguments.Count)
                {
                    parseError ??= new CommandLineParserParsingException($"Unexpected or unknown argument: '{currentArg}'");
                    return;
                }

                var plainDef = _plainArguments[currentPlainArgIndex];
                try
                {
                    object parsedValue = plainDef.GetParameter(currentArg);
                    parsedPlainArgs[plainDef] = parsedValue;
                }
                catch (Exception ex)
                {
                    parseError ??= ex;
                }
                finally
                {
                    currentPlainArgIndex++;
                }
            }
        }

        private sealed class DefaultOptionApplier
        {
            private readonly List<OptionDefinition> _options;

            public DefaultOptionApplier(List<OptionDefinition> options)
            {
                _options = options;
            }

            public void ApplyDefaults(Dictionary<OptionDefinition, CommandParserOptionResult> parsedOptions)
            {
                foreach (var option in _options)
                {
                    if (parsedOptions.ContainsKey(option)) continue;
                    if (!option.ParameterDefinitions.Any()) continue;

                    Dictionary<string, object> defaultParameters = new();
                    bool allDefaultsSuccess = true;

                    foreach (var paramDef in option.ParameterDefinitions)
                    {
                        try
                        {
                            object defaultValue = paramDef.GetParameter(null);
                            defaultParameters[paramDef.GetName()] = defaultValue;
                        }
                        catch
                        {
                            allDefaultsSuccess = false;
                            break;
                        }
                    }

                    if (allDefaultsSuccess)
                    {
                        parsedOptions[option] = new CommandParserOptionResult(option, defaultParameters, null);
                    }
                }
            }
        }

        private sealed class MissingRequirementsChecker
        {
            private readonly List<OptionDefinition> _options;
            private readonly List<IParameterDefinition> _plainArguments;

            public MissingRequirementsChecker(List<OptionDefinition> options, List<IParameterDefinition> plainArguments)
            {
                _options = options;
                _plainArguments = plainArguments;
            }

            public void CheckPlainArguments(int currentPlainArgIndex, ref Exception? parseError)
            {
                var missingRequired = _plainArguments
                    .Skip(currentPlainArgIndex)
                    .FirstOrDefault(arg => arg.IsRequired);

                if (missingRequired != null)
                {
                    parseError ??= new CommandLineParserException(
                        $"Missing required argument: {missingRequired.GetName()}");
                }
            }

            public void CheckRequiredOptions(Dictionary<OptionDefinition, CommandParserOptionResult> parsedOptions, ref Exception? parseError)
            {
                var missingRequired = _options.FirstOrDefault(option =>
                    !parsedOptions.ContainsKey(option) && option.ParameterDefinitions.Any(p => p.IsRequired));

                if (missingRequired != null)
                {
                    parseError ??= new CommandLineParserException(
                        $"Missing required option: {missingRequired.Aliases.First()}");
                }
            }
        }
    }
}
