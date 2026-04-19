import { Navigate, Route, Routes } from 'react-router-dom'
import { useAuth } from './store/AuthContext'
import Landing    from './pages/Landing'
import Login      from './pages/Login'
import Register   from './pages/Register'
import Onboarding from './pages/Onboarding'
import Discover   from './pages/Discover'
import Matches    from './pages/Matches'
import Profile    from './pages/Profile'
import NavBar     from './components/NavBar'

function Protected({ children }) {
  const { user, loading } = useAuth()
  if (loading) return <Splash />
  if (!user)   return <Navigate to="/" replace />
  return children
}

function Splash() {
  return (
    <div className="h-dvh flex items-center justify-center">
      <span className="text-3xl font-bold tracking-tight2 text-apple-text">Ditto</span>
    </div>
  )
}

export default function App() {
  const { user, loading } = useAuth()

  if (loading) return <Splash />

  return (
    <div className="max-w-md mx-auto h-dvh flex flex-col relative overflow-hidden">
      <Routes>
        <Route path="/"           element={user ? <Navigate to="/discover" replace /> : <Landing />} />
        <Route path="/login"      element={user ? <Navigate to="/discover" replace /> : <Login />} />
        <Route path="/register"   element={user ? <Navigate to="/discover" replace /> : <Register />} />
        <Route path="/onboarding" element={<Protected><Onboarding /></Protected>} />
        <Route path="/discover"   element={<Protected><Discover /></Protected>} />
        <Route path="/matches"    element={<Protected><Matches /></Protected>} />
        <Route path="/profile"    element={<Protected><Profile /></Protected>} />
      </Routes>

      {user && <NavBar />}
    </div>
  )
}
