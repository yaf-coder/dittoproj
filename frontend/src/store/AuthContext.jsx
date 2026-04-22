import { createContext, useCallback, useContext, useEffect, useState } from 'react'
import { api } from '../api/client'

const AuthContext = createContext(null)

export function AuthProvider({ children }) {
  const [user, setUser]       = useState(null)
  const [loading, setLoading] = useState(true)

  const loadUser = useCallback(async () => {
    const t = localStorage.getItem('ditto_token')
    if (!t) { setLoading(false); return }
    try {
      const me = await api.getMe()
      setUser(me)
    } catch {
      localStorage.removeItem('ditto_token')
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => { loadUser() }, [loadUser])

  const login = useCallback((token) => {
    localStorage.setItem('ditto_token', token)
    return loadUser()
  }, [loadUser])

  const logout = useCallback(() => {
    localStorage.removeItem('ditto_token')
    setUser(null)
  }, [])

  return (
    <AuthContext.Provider value={{ user, setUser, loading, login, logout }}>
      {children}
    </AuthContext.Provider>
  )
}

export function useAuth() {
  return useContext(AuthContext)
}
