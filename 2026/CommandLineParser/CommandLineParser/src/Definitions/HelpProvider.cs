namespace CommandLineParser
{
    /// <summary>
    /// Provides access to long and short help text for user assistance.
    /// </summary>
    public class HelpProvider
    {
        /// <summary>
        /// Initializes a new instance of the HelpProvider class with specified long and short help text.
        /// </summary>
        /// <param name="longHelpText">The detailed help text to display.</param>
        /// <param name="shortHelpText">The brief help text to display.</param>
        public HelpProvider(string longHelpText = "", string shortHelpText = "") 
        {
            _longHelpText = longHelpText;
            _shortHelpText = shortHelpText;
        }

        /// <summary>
        /// Retrieves the detailed help text for the command.
        /// </summary>
        /// <returns>A string containing the long-form help text.</returns>
        public virtual string GetLongHelpText() 
        { 
            return _longHelpText; 
        }

        /// <summary>
        /// Retrieves a short help text description.
        /// </summary>
        /// <returns>A short help text string.</returns>
        public virtual string GetShortHelpText() 
        { 
            return _shortHelpText; 
        }

        private readonly string _longHelpText;
        private readonly string _shortHelpText;
    }

    internal interface IHelpInfo
    {
        string longDescription { get; }
        string shortDescription { get; }
    }

    /// <summary>
    /// This record of the option is used for the users, to read this info and create appropriate help messages
    /// </summary>
    public sealed record OptionHelpInfo(
        string name,
        string longDescription, 
        string shortDescription, 
        IReadOnlyList<string> aliases,
        IReadOnlyList<ParameterHelpInfo> parameterNames
    ) : IHelpInfo;
    
    /// <summary>
    /// This record of the parameter is used for the users, to read this info and create appropriate help messages
    /// </summary>
    public sealed record ParameterHelpInfo(string name, string longDescription, string shortDescription) : IHelpInfo;

    /// <summary>
    /// This record of the user's program is used for the users, to read this info and create appropriate help messages
    /// </summary>
    public sealed record ProgramHelpInfo(string longDescription, string shortDescription) : IHelpInfo;

}