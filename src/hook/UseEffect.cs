using System;
using System.Collections.Generic;
using System.Linq;

namespace MikoInstaller.Hook;

public class UseEffect
{
    private readonly List<IDisposable> _disposables = new List<IDisposable>();
    private readonly Dictionary<string, object> _dependencies = new Dictionary<string, object>();
    
    public void Effect(Action effect, params object[] dependencies)
    {
        var dependencyKey = string.Join("|", dependencies != null ? dependencies.Select(d => d != null ? d.ToString() : "null") : new string[0]);
        
        if (!_dependencies.ContainsKey(dependencyKey) || 
            (_dependencies[dependencyKey] != null && !_dependencies[dependencyKey].Equals(dependencyKey)))
        {
            _dependencies[dependencyKey] = dependencyKey;
            effect.Invoke();
        }
    }
    
    public void Effect(Func<IDisposable> effect, params object[] dependencies)
    {
        var dependencyKey = string.Join("|", dependencies != null ? dependencies.Select(d => d != null ? d.ToString() : "null") : new string[0]);
        
        if (!_dependencies.ContainsKey(dependencyKey) || 
            (_dependencies[dependencyKey] != null && !_dependencies[dependencyKey].Equals(dependencyKey)))
        {
            _dependencies[dependencyKey] = dependencyKey;
            var disposable = effect.Invoke();
            if (disposable != null)
            {
                _disposables.Add(disposable);
            }
        }
    }
    
    public void Cleanup()
    {
        foreach (var disposable in _disposables)
        {
            disposable.Dispose();
        }
        _disposables.Clear();
        _dependencies.Clear();
    }
}