import { Navigate, Route, Routes, useLocation } from 'react-router-dom'
import { useAuth } from './store/AuthContext'
import Landing    from './pages/Landing'
import Login      from './pages/Login'
import Register   from './pages/Register'
import Onboarding from './pages/Onboarding'
import Questions  from './pages/Questions'
import Prompts    from './pages/Prompts'
import Discover   from './pages/Discover'
import Matches    from './pages/Matches'
import Profile    from './pages/Profile'
import NavBar     from './components/NavBar'

const TABBED_PATHS = ['/discover', '/prompts', '/matches', '/profile']

function Protected({ children }) {
  const { user, loading } = useAuth()
  if (loading) return <Splash />
  if (!user)   return <Navigate to="/" replace />
  return children
}

function Splash() {
  return (
    <div className="h-dvh flex items-center justify-center bg-apple-gray">
      <span className="text-4xl font-cursive text-apple-blue">Concordia</span>
    </div>
  )
}

function BrandHeader() {
  const location = useLocation()
  if (!TABBED_PATHS.includes(location.pathname)) return null
  return (
    <div className="shrink-0 px-5 pb-2 safe-top bg-apple-gray flex items-baseline gap-2 border-b border-apple-gray-2">
      <span className="font-cursive text-apple-blue text-4xl leading-none">Concordia</span>
      <span className="text-[11px] text-apple-sub/80 leading-none">Built on alignment, not attraction.</span>
    </div>
  )
}

export default function App() {
  const { user, loading } = useAuth()

  if (loading) return <Splash />

  return (
    <div className="w-full h-dvh flex flex-col relative overflow-hidden bg-apple-gray">
      {user && <BrandHeader />}
      <Routes>
        <Route path="/"           element={user ? <Navigate to="/discover" replace /> : <Landing />} />
        <Route path="/login"      element={<Login />} />
        <Route path="/register"   element={<Register />} />
        <Route path="/onboarding" element={<Protected><Onboarding /></Protected>} />
        <Route path="/questions"  element={<Protected><Questions /></Protected>} />
        <Route path="/prompts"    element={<Protected><Prompts /></Protected>} />
        <Route path="/discover"   element={<Protected><Discover /></Protected>} />
        <Route path="/matches"    element={<Protected><Matches /></Protected>} />
        <Route path="/profile"    element={<Protected><Profile /></Protected>} />
      </Routes>

      {user && <NavBar />}
    </div>
  )
}
