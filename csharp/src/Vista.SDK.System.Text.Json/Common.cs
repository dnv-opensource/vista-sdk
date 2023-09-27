﻿namespace Vista.SDK.Transport;

internal static class Common
{
    internal static IDictionary<string, object> CopyProperties(this IReadOnlyDictionary<string, object> props) =>
        props.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);

    internal static IReadOnlyDictionary<string, object> CopyProperties(this IDictionary<string, object> props) =>
        props.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);
}
